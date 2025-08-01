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
      return Result<TYPE>(rc, MSG);         \
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
      return Res(r.rc(), MSG); \
    }                               \
  }

#define TR(VAL)                                                \
  {                                                            \
    auto r = (VAL);                                            \
    if (r.is_err())                                            \
    {                                                          \
      INFO("[%d] %s:%d %s", r.rc(), __FILE__, __LINE__, #VAL); \
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
      return Res(rc, #VAL);                             \
    }                                                        \
  }

#define CHECK_ESP(VAL)                                                       \
  {                                                                          \
    esp_err_t rc = (VAL);                                                    \
    if (rc != ESP_OK)                                                        \
    {                                                                        \
      const char *erc_str = esp_err_to_name(rc);                             \
      ERROR("rc=%d:%s %s:%d %s", rc, erc_str, __FILE__, __LINE__, #VAL); \
      return Res(rc, erc_str);                                          \
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
#define FNV_PRIME 16777619
#define FNV_OFFSET 2166136261
#define FNV_MASK 0xFFFF
#endif


#if FNV_LENGTH == 32
#define FNV_PRIME 16777619
#define FNV_OFFSET 2166136261
#define FNV_MASK 0xFFFFFFFFu
#endif

#if FNV_LENGTH == 64
#define FNV_PRIME 1099511628211ull
#define FNV_OFFSET 14695981039346656037ull
#endif

constexpr uint32_t fnv1(uint32_t h, const char *s)
{
    return (*s == 0) ? h
           : fnv1((h * FNV_PRIME) ^ static_cast<uint32_t>(*s), s + 1);
}

constexpr uint16_t H(const char *s)
{
    //    uint32_t  h = fnv1(FNV_OFFSET, s) ;
    return (fnv1(FNV_OFFSET, s) & FNV_MASK);
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
