#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <log.h>
#include <errno.h>
#include <cstring>
#include <option.h>
#include <result.h>

//#include "esp_err.h"


#define BZERO(x) memset(&(x), 0, sizeof(x))
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)
#define FILE_LINE __FILE__ ":" LINE_STRING " "
#define PANIC(S) panic_here(__FILE__ ":" LINE_STRING " " S)
#define LOG(S) { printf(__FILE__ ":" LINE_STRING " " S "\n");fflush(stdout);}


// #include <ArduinoJson.h>

typedef std::vector<uint8_t> Bytes;

#define TEST_RC(TYPE, VAL, MSG)                  \
  {                                              \
    auto rc = (VAL);                             \
    if (rc < 0)                                  \
    {                                            \
      INFO(MSG);                                 \
      INFO("=%d %s:%d", rc, __FILE__, __LINE__); \
      return Result<TYPE>::Err(rc, MSG);         \
    }                                            \
  }
#define TEST_R(TYPE, VAL, MSG)                       \
  {                                                  \
    auto r = (VAL);                                  \
    if (r.is_err())                                  \
    {                                                \
      INFO(MSG);                                     \
      INFO("=%d %s:%d", r.rc(), __FILE__, __LINE__); \
      return Result<TYPE>(r.rc(), MSG);         \
    }                                                \
  }
#define RET_ERR(VAL, MSG)           \
  {                                 \
    auto r = (VAL);                 \
    if (r.is_err())                 \
    {                               \
      return Res::Err(r.err()->rc, MSG); \
    }                               \
  }

#define TR(VAL)                                                \
  {                                                            \
    auto r = (VAL);                                            \
    if (r.is_err())                                            \
    {                                                          \
      INFO("[%d] %s:%d %s", r.err()->rc(), __FILE__, __LINE__, #VAL); \
      return r;                                                \
    }                                                          \
  }

#define T_ESP(VAL)                                                                       \
  {                                                                                      \
    auto r = (VAL);                                                                      \
    if (r != ESP_OK)                                                                     \
    {                                                                                    \
      INFO("[%d] %s:%d %s = %s ", r, __FILE__, __LINE__, #VAL, esp_err_to_name(r)); \
      return r;                                                                          \
    }                                                                                    \
  }

#define RET_ERRI(VAL, MSG)                        \
  {                                               \
    int rc = (VAL);                               \
    if (rc < 0)                                   \
    {                                             \
      ERROR(MSG);                                  \
      ERROR("[%d] %s:%d", rc, __FILE__, __LINE__); \
      return Res(rc, MSG);                   \
    }                                             \
  }

#define ERRNO(VAL) \
{                                               \
  int rc = (VAL);                               \
  if (rc < 0)                                   \
  {                                             \
    ERROR("[%d] %s:%d failed " #VAL " with %d:%s", rc, __FILE__, __LINE__,errno,strerror(errno)); \
  }                                             \
}



#define CHECK(VAL)                                           \
  {                                                          \
    int rc = (VAL);                                          \
    if (rc != 0)                                             \
    {                                                        \
      ERROR("rc=%d %s:%d %s", rc, __FILE__, __LINE__, #VAL); \
      return Res::Err(rc, #VAL);                             \
    }                                                        \
  }

#define CHECK_ESP(VAL)                                                       \
  {                                                                          \
    esp_err_t rc = (VAL);                                                    \
    if (rc != ESP_OK)                                                        \
    {                                                                        \
      const char *erc_str = esp_err_to_name(rc);                             \
      ERROR("rc=%d:%s %s:%d %s", rc, erc_str, __FILE__, __LINE__, #VAL); \
      return Res::Err(rc, erc_str);                                          \
    }                                                                        \
  }


template <typename T>
void for_each(std::optional<T> &opt, std::function<void()> action)
{
  if (opt.has_value())
  {
    action();
  }
}

#define FNV_LENGTH 16

#ifndef FNV_LENGTH
#define FNV_LENGTH 16
#endif

#if FNV_LENGTH == 16
#define FNV16_PRIME 16777619
#define FNV16_OFFSET 2166136261
#define FNV16_MASK 0xFFFF
#endif


#define FNV32_PRIME 16777619
#define FNV32_OFFSET 2166136261
#define FNV32_MASK 0xFFFFFFFFu

#define FNV64_PRIME 1099511628211ull
#define FNV64_OFFSET 14695981039346656037ull
#define FNV64_MASK 0xFFFFFFFFFFFFFFFFull

#define FILE_LINE_STR ( __FILE__ ":" STRINGIZE(__LINE__) " " )
#define FILE_LINE_HASH ( H(__FILE__ ":" STRINGIZE(__LINE__)) )

constexpr uint32_t fnv32(uint32_t h, const char *s)
{
    return (*s == 0) ? h
           : fnv32((h * FNV32_PRIME) ^ static_cast<uint32_t>(*s), s + 1);
}

constexpr uint16_t H(const char *s)
{
    return (fnv32(FNV32_OFFSET, s) & FNV32_MASK);
}

template <typename T, typename U>
std::optional<U> map(std::optional<T> t, std::function<U(T)> f)
{
  if (t)
  {
    return f(t);
  }
  else
  {
    return std::nullopt;
  }
}

template <typename T, typename F>
void operator>>(std::optional<T> t, F f)
{
  if (t)
  {
    f(t.value());
  }
}

template <typename T, typename F>
auto operator>>=(const std::optional<T> &opt, F &&func)
    -> std::optional<std::invoke_result_t<F, T>>
{
  // If the optional has a value, apply the function to it and wrap the result in an optional
  if (opt)
  {
    return std::invoke(std::forward<F>(func), *opt);
  }
  // Otherwise, return an empty optional
  return std::nullopt;
}

#endif
