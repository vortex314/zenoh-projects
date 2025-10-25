#ifndef RESULT_H
#define RESULT_H

#include <iostream>
#include <stdio.h>
#include <string>
#include <utility>
#include <variant>
#include <stdint.h>
#include <util.h>

// void panic_here(const char *s){ printf(" ===> PANIC : %s\n", s);fflush(stdout); }

#define VAL_OR_RET(L, F)           \
    {                              \
        auto v = F;                \
        if (v.is_err())            \
            return v;              \
        L = v.unwrap(); \
    }

typedef struct Error 
{
    int code;
    std::string message;
} Error;

template <typename T>
class Result
{
private:
    std::variant<T,Error> _value; // Use variant to hold either a value or an error

public:
    // Constructor for success
    Result(T value)
    {
        _value = std::move(value);
    }

    Result(int rc, const char *msg)
    {
        _value = Error{rc, std::string(msg)};
    }

    template <typename U>
    Result(Result<U> r)
    {
        if (r.is_ok())
        {
            _value = (T)r.unwrap();
        }
        else
        {
            _value = Error{r.rc(), r.msg()};
        }
    }

    Result()
    {
        _value = Error{0, "No value"};
    }

    ~Result()
    {
        // No need to manually manage memory for std::variant
        // It will automatically clean up the contained value or error
        // if (_value.index() == 1) // If it holds an Error
        // {
        //     auto &err = std::get<Error>(_value);
        //     // No need to delete, std::string will handle its own memory
        // }
        // else if (_value.index() == 0) // If it holds a T
        // {
        //     auto &val = std::get<T>(_value);
        //     // No need to delete, T will handle its own memory if it's a pointer
        // }

    }

    inline bool is_ok() const
    {
        return std::holds_alternative<T>(_value);
    }

    // Check if the result is an error
    inline bool is_err() const
    {
        return std::holds_alternative<Error>(_value);
    }

    // Get the value (T), if OK
    T unwrap() const noexcept
    {
        if (is_err())
        {
            PANIC("Attempted to unwrap an error result");
        }
        return std::get<T>(_value);
    }

    const T &ref() const
    {
        if (is_err())
        {
            PANIC("Attempted to ref an error result");
        }
        return std::get<T>(_value);
    }

    Option<T> ok() const
    {
        if (is_ok())
        {
            return Option<T>(ref());
        }
        return Option<T>(nullptr);
    }

    // Get the error (E), if there's an error
    inline int rc() const noexcept
    {
        if (is_err())
        {
            return std::get<Error>(_value).code;
        }
        return 0; // No error code for success
    }

    inline const char *msg() const noexcept
    {
        if (is_err())
        {
            return std::get<Error>(_value).message.c_str();
        }
        return "No error";
    }

    template <typename F>
    auto inspect(F &&func) const -> Result<T>
    {
        if (is_ok())
        {
            func(ref());
        }
        return *this;
    }

    template <typename F>
    auto on_error(F &&func) const -> Result<T>
    {
        if (is_err())
            func(std::get<Error>(_value).code, std::get<Error>(_value).message.c_str());
        return *this;
    }

    template <typename F>
    auto map(F &&func) const -> Result<decltype(func(std::declval<T>()))>
    {
        using ResultType = decltype(func(std::declval<T>()));
        if (is_ok())
        {
            return Result<ResultType>(func(ref()));
        }
        return Result<ResultType>(rc(),msg());
    }
    template <typename F>
    auto and_then(F &&func) const -> Result<decltype(func(std::declval<T>()))>
    {
        using ResultType = decltype(func(std::declval<T>()));
        if (is_ok())
        {
            return Result<ResultType>(func(ref()));
        }
        return Result<ResultType>(rc(),msg());
    }
    template <typename F>
    auto filter(F &&predicate) const -> Result<T>
    {
        if (is_ok() && predicate(ref()))
        {
            return Result<T>(ref()); // Return the current value if predicate is true
        }
        return Result<T>(ENOTCONN, "filter failed"); // Return the current error if predicate fails
    }
};

typedef bool Void;
typedef Result<Void> Res;
// #define ResOk Res(true)
// constexpr Res ResOk = Result<Void>(true);
static Res ResOk = Result<Void>(true);
/*
typedef union Un {
        union Un* _pv;
        std::string* _msg;
    } Un;

int main() {
    Result<int> success(42);
    Result<int> error(-13,"An error occurred");
    printf("--------------> %lu\n",sizeof(success));
printf("--------------> %lu\n",sizeof(Un));

    Result<long unsigned int> new_success = success.map([](int value) {
       printf(" value : %d \n",value);
        return "hello world" ;
    }).map([](std::string value) {
       printf(" value : %s \n",value.c_str());
      return value.length();
    });
     LOG("");
    printf(" res %lu \n",new_success.ref());

    printf("======== GAME OVER ========== \n");

    return 0;
}*/
#endif // RESULT_H