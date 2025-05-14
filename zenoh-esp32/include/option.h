#ifndef _OPTION_H_
#define _OPTION_H_
#include <stdint.h>
#include <stdio.h>

#include <functional>
#include <string>
#include <utility>

void panic_here(const char *s); //{ printf(" ===> PANIC : %s\n", s); }

#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#define PANIC(S) panic_here(__FILE__ ":" LINE_STRING " " S)

/*
 Trying the equivalent of Option in Rust
*/
template <typename T>
class Option
{
private:
    T *_pv;

public:
    inline Option()
    {
        _pv = nullptr;
    }
    inline Option(T t)
    {
        _pv = new T;
        *_pv = t;
    }
    inline Option(nullptr_t)
    {
        _pv = nullptr;
    }
    ~Option()
    {
        if (_pv)
        {
            delete _pv;
        }
    }
    bool operator==(const T t) const
    {
        if (_pv)
        {
            return *_pv == t;
        }
        else
        {
            return false;
        }
    }

    const T *operator->() const
    {
        if (_pv == nullptr)
            PANIC("Option empty");
        return _pv;
    }
    const T &operator*() const
    {
        if (_pv == nullptr)
            PANIC("Option empty");
        return *_pv;
    }
    constexpr explicit operator bool() const noexcept { return _pv != nullptr; }
    constexpr void operator=(const T t)
    {
        if (_pv == nullptr)
        {
            _pv = new T;
        }
        *_pv = t;
    }
    constexpr Option<T> &operator=(const Option<T> &other)

    {
        if (other._pv == nullptr)
        {
            if (_pv != nullptr)
            {
                delete _pv;
            }
            _pv = nullptr;
            return *this;
        }
        if (_pv == nullptr)
        {
            _pv = new T;
        }
        *_pv = *other._pv;
        return *this;
    }
    Option<T> &operator=(Option<T> &&other)
    {
        if (other._pv == nullptr)
        {
            _pv = nullptr;
            return *this;
        }
        _pv = other._pv;
        other._pv = nullptr;
        return *this;
    }

    template <typename U>
    Option<U> map(std::function<Option<U>(const T &)> f)
    {
        if (_pv)
        {
            return f(*_pv);
        }
        else
        {
            return Option<U>();
        }
    }

    inline bool is_some() const {
        return _pv != nullptr;
    }

    inline bool is_none() const {
        return _pv == nullptr;
    }

    inline const T& ref() const {
        if (is_none()) {
            PANIC("Attempted to unwrap an error result");
        }
        return *_pv;
    }
    // map ((T) -> U) -> Option<U>
    template <typename F>
    auto map(F&& func) const -> Option<decltype(func(std::declval<T>()))> {
        using OptionType = decltype(func(std::declval<T>()));
        if (is_some()) {
            return func(ref());
        }
        return Option<OptionType>();
    }
    // and_then ((T) -> Option<U>) -> Option<U>
    template <typename F>
    auto and_then(F&& func) const -> Option<decltype(func(std::declval<T>()))> {
        using OptionType = decltype(func(std::declval<T>()));
        if (is_some()) {
            return Option<OptionType>(func(ref()));
        }
        return Option<OptionType>();
    }
    template <typename F>
    auto filter(F&& predicate) const -> Option<T>  {
        if (is_some() && predicate(ref())) {
            return Option<T>(ref()); // Return the current value if predicate is true
        }
        return Option<T>(); // Return None if predicate fails
    }
    template <typename F>
    auto inspect(F&& predicate) const -> Option<T>  {
        predicate(ref());
        return *this; 
    }

    template <typename F>
    void for_each(F&& func) const  {
        if (is_some() ) func(ref());
    }
    Option(const Option<T> &other)
    {
        if (other._pv == nullptr)
        {
            _pv = nullptr;
            return;
        }
        _pv = new T;
        *_pv = *other._pv;
    }
    void operator>>(std::function<void(  T &)> f) const
    {
        if (_pv)
            f(*_pv);
    }
    template <typename U>
    Option<U> operator>>(std::function<Option<U>(const T &)> f)
    {
        return _pv ? f(*_pv) : nullptr;
    }

    template <typename U,typename F>
    Option<U> operator<<(F&& f)
    {
        return _pv ? f(*_pv) : nullptr;
    }

    template <typename U>
    Option<U> map(std::function<Option<U>(T)> f)
    {
        return _pv ? f(*_pv) : nullptr;
    }

    /*const Option<T> filter(std::function<bool(T)> f)
    {
        if (_pv == nullptr)
            return *this;
        if (f(*_pv))
        {
            return *this;
        }
        else
            return nullptr;
    }*/

    const T &value() const
    {
        if (_pv == nullptr)
            PANIC("");
        return *_pv;
    }
};
/*
int main() {
    Option<std::string> opt_str1;
    PANIC("Life is Hard ");
    opt_str1 = "aa";
    Option<std::string> opt_str2;
    opt_str2 = std::move(opt_str1);

    if (!opt_str1) {
        printf(" No value yet \n");
        fflush(stdout);
    } else {
        printf(" Value opt_str1 : %s \n", (*opt_str1).c_str());
    }
    if (!opt_str2) {
        printf(" No value yet \n");
        fflush(stdout);
    } else {
        printf(" Value opt_str2: %s \n", (*opt_str2).c_str());
    }

    auto v = Option<int>(10);
    auto u_opt = opt_str2.map<int>(
        [&](const std::string& t) { return Option<int>(111); });
    if (u_opt) {
        printf(" u_opt : %d \n", *u_opt);
        fflush(stdout);
    } else {
        printf(" u_opt is empty \n");
        fflush(stdout);
    }
}
    */
#endif