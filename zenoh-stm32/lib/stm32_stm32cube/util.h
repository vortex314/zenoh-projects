
#ifndef __UTIL_H__
#define __UTIL_H__
#include <variant>
#include <string>
#include <vector>
#include <optional>
#include <functional>

extern "C" void panic_handler(const char *msg);


template <typename T>
class Opt : std::optional<T>
{
    Opt(T t) : std::optional<T>(t) {};
    Opt() : std::optional<T>() {};
    bool is_some() { return this->has_value(); }
    bool is_none() { return !this->has_value(); }
    T unwrap() { return this->value(); }
    T unwrap_or(T def) { return this->has_value() ? this->value() : def; }
    T expect(const char *msg)
    {
        if (is_none())
        {
            panic_handler("Opt::expect");
        }
        return unwrap();
    };
    template <typename U>
    Opt<U> map(U (*f)(T)) { return is_some() ? Opt<U>(f(unwrap())) : Opt<U>(); }
    template <typename U>
    Opt<U> and_then(Opt<U> (*f)(T)) { return is_some() ? f(unwrap()) : Opt<U>(); }
    template <typename U>
    Opt<U> or_else(Opt<U> (*f)(T)) { return is_some() ? Opt<U>() : f(unwrap()); }
    template <typename U>
    Opt<U> map_or(U def, U (*f)(T)) { return is_some() ? f(unwrap()) : def; }
    template <typename U>
    Opt<U> map_or_else(U (*def)(T), U (*f)(T)) { return is_some() ? f(unwrap()) : def(unwrap()); }
};

typedef struct Error
{
    int code;
    const char *msg;
} Error;


template <typename T>
class Res : public std::variant<T, Error>
{
public:
    Res() : std::variant<T, Error>() {};
    Res(T t) : std::variant<T, Error>(t) {};
    Res(Error e) : std::variant<T, Error>(e) {};
    Res<T> ok(T t)
    {
        *this = t;
        return *this;
    }
    bool is_ok() { return std::holds_alternative<T>(*this); }
    bool is_err() { return std::holds_alternative<Error>(*this); }
    Res<T> err(Error e)
    {
        *this = e;
        return *this;
    }

    T unwrap() { return std::get<T>(*this); }
    Error unwrap_err() { return std::get<Error>(*this); }
    T expect(const char *msg)
    {
        if (is_err())
        {
            panic_handler("Res::expect");
        }
        return unwrap();
    }
    Error expect_err(const char *msg)
    {
        if (is_ok())
        {
            panic_handler("Res::expect_err");
        }
        return unwrap_err();
    }
    template <typename U>
    Res<U> map(std::function<Res<U>(T)> f) { return is_ok() ? Res<U>::ok(f(unwrap())) : Res<U>::err(unwrap_err()); };
    void and_then(std::function<void(T)> f)
    {
        if (is_ok())
            f(unwrap());
    }
    template <typename U>
    Res<U> map_err(U (*f)(Error))
    {
        return is_err() ? Res<U>::err(f(unwrap_err())) : Res<U>::ok(unwrap());
    }
    template <typename U>
    Res<U> or_else(Res<U> (*f)(Error))
    {
        return is_err() ? f(unwrap_err()) : Res<U>::ok(unwrap());
    }
};

#endif // __UTIL_H__