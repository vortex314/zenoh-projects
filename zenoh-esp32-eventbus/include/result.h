
// result.hpp
#pragma once
#include <optional>
#include <type_traits>
#include <utility>
#include <cstdlib>   // std::abort
#include <iostream>  // optional debug printing
#include <stdint.h>

// ---------------------------------------------------------------------------
// Configurable panic handler (default = abort)
namespace result_detail {
    using panic_fn = void(*)(const char*);
    inline panic_fn g_panic = [](const char* msg) {
        // You can replace this with a custom handler at runtime:
        //   result_detail::set_panic(my_handler);
        (void)msg;
        std::abort();
    };

    inline void set_panic(panic_fn f) noexcept { g_panic = f; }
    [[noreturn]] inline void panic(const char* msg) { g_panic(msg); while(1){};}
}

typedef struct Error {
    int32_t rc;
    const char* msg;
    bool operator==(const Error& other) const noexcept {
        return rc == other.rc && msg == other.msg;
    }
} Error;

// ---------------------------------------------------------------------------
template<class T>
class [[nodiscard]] Result {
    enum class Tag { Ok, Err };
    Tag tag_;

    union {
        T ok_;
        Error err_;
    };

    // -----------------------------------------------------------------------
    // Helpers
    static constexpr bool nothrow_copy =
        std::is_nothrow_copy_constructible_v<T>;

    static constexpr bool nothrow_move =
        std::is_nothrow_move_constructible_v<T>;

    // -----------------------------------------------------------------------
    // Construction helpers
    constexpr void destroy() noexcept {
        if (tag_ == Tag::Ok)  ok_.~T();
        else                  err_.~Error();
    }

    constexpr void copy_from(const Result& other) noexcept(nothrow_copy) {
        if (other.tag_ == Tag::Ok)  new (&ok_)  T(other.ok_);
        else                        new (&err_) Error(other.err_);
        tag_ = other.tag_;
    }

    constexpr void move_from(Result&& other) noexcept(nothrow_move) {
        if (other.tag_ == Tag::Ok)  new (&ok_)  T(std::move(other.ok_));
        else                        new (&err_) Error(std::move(other.err_));
        tag_ = other.tag_;
    }

public:
    // -----------------------------------------------------------------------
    // Constructors
    template<class U = T,
             class = std::enable_if_t<std::is_constructible_v<T, U&&>>>
    constexpr Result(U&& v) noexcept(std::is_nothrow_constructible_v<T, U&&>)
        : tag_(Tag::Ok), ok_(std::forward<U>(v)) {}

    template<class V = Error,
             class = std::enable_if_t<std::is_constructible_v<Error, V&&>>>
    constexpr Result(Error&& e) noexcept(std::is_nothrow_constructible_v<Error, V&&>)
        : tag_(Tag::Err), err_(std::forward<V>(e)) {}

    // Copy
    constexpr Result(const Result& other) noexcept(nothrow_copy) { copy_from(other); }
    constexpr Result& operator=(const Result& other) noexcept(nothrow_copy) {
        if (this != &other) { destroy(); copy_from(other); }
        return *this;
    }

    // Move
    constexpr Result(Result&& other) noexcept(nothrow_move) { move_from(std::move(other)); }
    constexpr Result& operator=(Result&& other) noexcept(nothrow_move) {
        if (this != &other) { destroy(); move_from(std::move(other)); }
        return *this;
    }

    // Destructor
    ~Result() noexcept { destroy(); }

    // -----------------------------------------------------------------------
    // Factory
    static constexpr Result Ok(T v)  noexcept(std::is_nothrow_move_constructible_v<T>)
        { return Result(std::move(v)); }
    static constexpr Result Err(Error e) noexcept(std::is_nothrow_move_constructible_v<Error>)
        { return Result(std::move(e)); }
    static constexpr Result Err(int rc,const char* msg) noexcept(std::is_nothrow_move_constructible_v<Error>)
        { Error e = { rc,msg };
            return Result(std::move(e)); }

    // -----------------------------------------------------------------------
    // Inspection
    [[nodiscard]] constexpr bool is_ok()  const noexcept { return tag_ == Tag::Ok; }
    [[nodiscard]] constexpr bool is_err() const noexcept { return tag_ == Tag::Err; }

    [[nodiscard]] constexpr bool contains(const T& v) const noexcept
        { return tag_ == Tag::Ok && ok_ == v; }
    [[nodiscard]] constexpr bool contains_err(const Error& e) const noexcept
        { return tag_ == Tag::Err && err_ == e; }

    // -----------------------------------------------------------------------
    // Panic-on-wrong-variant access
    [[nodiscard]] constexpr T expect(const char* msg) && {
        if (tag_ == Tag::Ok) return std::move(ok_);
        result_detail::panic(msg);
    }
    [[nodiscard]] constexpr const T& expect(const char* msg) const& {
        if (tag_ == Tag::Ok) return ok_;
        result_detail::panic(msg);
    }

    [[nodiscard]] constexpr Error expect_err(const char* msg) && {
        if (tag_ == Tag::Err) return std::move(err_);
        result_detail::panic(msg);
    }
    [[nodiscard]] constexpr const Error& expect_err(const char* msg) const& {
        if (tag_ == Tag::Err) return err_;
        result_detail::panic(msg);
    }

    [[nodiscard]] constexpr T unwrap() && {
        return std::move(*this).expect("unwrap() on Err");
    }
    [[nodiscard]] constexpr const T& unwrap() const& {
        return this->expect("unwrap() on Err");
    }

    [[nodiscard]] constexpr Error unwrap_err() && {
        return std::move(*this).expect_err("unwrap_err() on Ok");
    }
    [[nodiscard]] constexpr const Error& unwrap_err() const& {
        return this->expect_err("unwrap_err() on Ok");
    }

    // -----------------------------------------------------------------------
    // Safe access
    [[nodiscard]] constexpr std::optional<T> ok() const& {
        return tag_ == Tag::Ok ? std::optional<T>(ok_) : std::nullopt;
    }
    [[nodiscard]] constexpr std::optional<T> ok() && {
        return tag_ == Tag::Ok ? std::optional<T>(std::move(ok_)) : std::nullopt;
    }

    [[nodiscard]] constexpr std::optional<Error> err() const& {
        return tag_ == Tag::Err ? std::optional<Error>(err_) : std::nullopt;
    }
    [[nodiscard]] constexpr std::optional<Error> err() && {
        return tag_ == Tag::Err ? std::optional<Error>(std::move(err_)) : std::nullopt;
    }

    // -----------------------------------------------------------------------
    // unwrap_or / unwrap_or_else
    template<class U>
    [[nodiscard]] constexpr T unwrap_or(U&& def) && noexcept(
        std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_constructible_v<T, U&&>)
    {
        return tag_ == Tag::Ok ? std::move(ok_) : static_cast<T>(std::forward<U>(def));
    }

    template<class F>
    [[nodiscard]] constexpr T unwrap_or_else(F&& f) && noexcept(
        std::is_nothrow_move_constructible_v<T> &&
        std::is_nothrow_invocable_r_v<T, F, const Error&>)
    {
        return tag_ == Tag::Ok ? std::move(ok_) : static_cast<T>(f(err_));
    }

    // -----------------------------------------------------------------------
    // map / map_err
    template<class F>
    [[nodiscard]] constexpr auto map(F&& f) & -> Result<decltype(f(std::declval<T&>()))>
    {
        using R = decltype(f(std::declval<T&>()));
        if (tag_ == Tag::Ok) return Result<R>::Ok(f(ok_));
        return Result<R>::Err(err_);
    }

    template<class F>
    [[nodiscard]] constexpr auto map(F&& f) && -> Result<decltype(f(std::declval<T&&>()))>
    {
        using R = decltype(f(std::declval<T&&>()));
        if (tag_ == Tag::Ok) return Result<R>::Ok(f(std::move(ok_)));
        return Result<R>::Err(std::move(err_));
    }

    template<class F>
    [[nodiscard]] constexpr auto map_err(F&& f) & -> Result<T>
    {
        if (tag_ == Tag::Err) return Result<T>::Err(f(err_));
        return Result<T>::Ok(ok_);
    }

    template<class F>
    [[nodiscard]] constexpr auto map_err(F&& f) && -> Result<T>
    {
        if (tag_ == Tag::Err) return Result<T>::Err(f(std::move(err_)));
        return Result<T>::Ok(std::move(ok_));
    }

    // -----------------------------------------------------------------------
    // and_then / or_else (Rust's flat_map / or_else)
    template<class F>
    [[nodiscard]] constexpr auto and_then(F&& f) &
        -> decltype(f(std::declval<T&>()))
    {
        using R = decltype(f(std::declval<T&>()));
        static_assert(std::is_same_v<typename R::value_type, void> == false,
                      "and_then callback must return a Result");
        if (tag_ == Tag::Ok) return f(ok_);
        return R::Err(err_);
    }

    template<class F>
    [[nodiscard]] constexpr auto and_then(F&& f) &&
        -> decltype(f(std::declval<T&&>()))
    {
        using R = decltype(f(std::declval<T&&>()));
        if (tag_ == Tag::Ok) return f(std::move(ok_));
        return R::Err(std::move(err_));
    }

    template<class F>
    [[nodiscard]] constexpr auto or_else(F&& f) &
        -> decltype(f(std::declval<Error&>()))
    {
        using R = decltype(f(std::declval<Error&>()));
        if (tag_ == Tag::Err) return f(err_);
        return R::Ok(ok_);
    }

    template<class F>
    [[nodiscard]] constexpr auto or_else(F&& f) &&
        -> decltype(f(std::declval<Error&&>()))
    {
        using R = decltype(f(std::declval<Error&&>()));
        if (tag_ == Tag::Err) return f(std::move(err_));
        return R::Ok(std::move(ok_));
    }

    // -----------------------------------------------------------------------
    // Conversion to bool
    explicit constexpr operator bool() const noexcept { return is_ok(); }

    // -----------------------------------------------------------------------
    // Debug printing (optional, can be removed for size-critical code)
#if defined(RESULT_ENABLE_DEBUG_PRINT)
    void debug_print() const {
        if (tag_ == Tag::Ok)  std::cout << "Ok(" << ok_ << ")\n";
        else                  std::cout << "Err(" << err_ << ")\n";
    }
#endif
};

// ---------------------------------------------------------------------------
// Deduction guides (C++17)
template<class T>
Result(T) -> Result<std::decay_t<T>>;

template<class T, class E>
Result(T, E) -> Result<std::decay_t<T>>;

// ---------------------------------------------------------------------------
// Convenience free functions
template<class T, class E>
constexpr Result<T> Ok(T v) noexcept(std::is_nothrow_move_constructible_v<T>)
{ return Result<T>::Ok(std::move(v)); }

template<class T, class E>
constexpr Result<T> Err(E e) noexcept(std::is_nothrow_move_constructible_v<E>)
{ return Result<T>::Err(std::move(e)); }

typedef Result<bool> Res;