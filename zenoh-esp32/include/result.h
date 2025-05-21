#include <iostream>
#include <stdio.h>
#include <string>
#include <utility>
#include <variant>
#include <stdint.h>

// void panic_here(const char *s){ printf(" ===> PANIC : %s\n", s);fflush(stdout); }

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#define PANIC(S) panic_here(__FILE__ ":" LINE_STRING " " S)
#define LOG(S) { printf(__FILE__ ":" LINE_STRING " " S "\n");fflush(stdout);}

template <typename T>
class Result {
private:
    union {
        T* _pv;
        std::string* _msg;
    };
    int16_t _rc;

public:
    // Constructor for success
    Result(T value)  {
        _rc=0;
        _pv = new T;
        *_pv = value;
    }

    Result() {
        _rc = -1;
        _msg = new std::string("default -1 error");
    }
    
    ~Result() {
      if ( is_ok() ) delete _pv;
      else delete _msg;
    }

    Result(int rc, const char* msg)  {
        if ( rc ==0 ) PANIC(" error code should not be 0 with error msg");
        _rc = rc;
        _msg = new std::string(msg);
    }

    inline bool is_ok() const {
        return _rc == 0;
    }

    // Check if the result is an error
    inline bool is_err() const {
        return _rc != 0;
    }

    // Get the value (T), if OK
    T unwrap() const noexcept {
        if (is_err()) {
            PANIC("Attempted to unwrap an error result");
        }
        LOG("std::get");
        return *_pv;
    }
    
    const T& ref() const {
        if (is_err()) {
            PANIC("Attempted to unwrap an error result");
        }
        return *_pv;
    }

    Option<T> ok() const {
        if (is_ok()) {
            return Option<T>(ref());
        }
        return Option<T>(nullptr);
    }

    // Get the error (E), if there's an error
    inline int rc() const noexcept {
        return _rc;
    }

    inline const char* msg() const noexcept {
        if ( is_ok() ){
            return("Ok");
        };
        return _msg->c_str();
    }

        template <typename F>
    auto inspect(F &&func) const -> Result<T>
    {
        if (is_ok())
            func(ref());
        return *this;
    }

    template <typename F>
    auto on_error(F &&func) const -> Result<T>
    {
        if (is_err())
            func(_rc,_msg);
        return *this;
    }


    
    template <typename F>
    auto map(F&& func) const -> Result<decltype(func(std::declval<T>()))> {
        using ResultType = decltype(func(std::declval<T>()));
        if (is_ok()) {
            return Result<ResultType>(func(ref()));
        }
        return Result<ResultType>(_rc,_msg->c_str());
    }
    template <typename F>
    auto and_then(F&& func) const -> Result<decltype(func(std::declval<T>()))> {
        using ResultType = decltype(func(std::declval<T>()));
        if (is_ok()) {
            return Result<ResultType>(func(ref()));
        }
        return Result<ResultType>(_rc,_msg->c_str());
    }
    template <typename F>
    auto filter(F&& predicate) const -> Result<T> {
        if (is_ok() && predicate(ref())) {
            return Result<T>(ref()); // Return the current value if predicate is true
        }
        return Result<T>(ENOTCONN, "filter failed"); // Return the current error if predicate fails
    }
};
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
