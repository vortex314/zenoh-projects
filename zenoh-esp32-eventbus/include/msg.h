// Msg.hpp — C++17, header-only
#pragma once
#include <string_view>
#include <vector>
#include <functional>
#include <type_traits>
#include <result.h>
#include <stdint.h>
#include <util.h>

using Bytes = std::vector<uint8_t>;

typedef uint32_t MsgId;
// ————————————————————————————————————————
// 1. Non-virtual base (for polymorphic use)
// ————————————————————————————————————————
struct Msg {
    virtual ~Msg() = default;
    // Runtime ID (fast)
    public:
    virtual MsgId type_id() const = 0;
    virtual const char* type_name() const = 0;

    // Functional dispatch
    template<typename T, typename F>
    void handle(F&& f) const {
        if (type_id() == T::id_value) {
            f(static_cast<const T&>(*this));
        }
    }

protected:
    template<typename T> struct Derived;
};

#define MSG(Name) \
public: \
    static constexpr const char* name_value = #Name; \
    static constexpr MsgId id_value = fnv32(FNV32_OFFSET, #Name); \
    inline MsgId type_id() const noexcept override { return id_value; }; \
    inline const char* type_name() const noexcept override { return #Name; }; \
    ~Name() override = default; 

#define DEFINE_MSG(Name,...) \
struct Name : public Msg { \
    MSG(Name) \
    __VA_ARGS__; \
    Name() = default; \
};

