#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <log.h>

#include "esp_err.h"

// #include <ArduinoJson.h>

typedef std::vector<uint8_t> Bytes;
typedef bool Void;
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
      return Result<TYPE>::Err(r.rc(), MSG);         \
    }                                                \
  }
#define RET_ERR(VAL, MSG)           \
  {                                 \
    auto r = (VAL);                 \
    if (r.is_err())                 \
    {                               \
      return Res::Err(r.rc(), MSG); \
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
      INFO("[%d] %s:%d %s = %s ", r.rc(), __FILE__, __LINE__, #VAL, esp_err_to_name(r)); \
      return r;                                                                          \
    }                                                                                    \
  }

#define RET_ERRI(VAL, MSG)                        \
  {                                               \
    int rc = (VAL);                               \
    if (rc < 0)                                   \
    {                                             \
      INFO(MSG);                                  \
      INFO("[%d] %s:%d", rc, __FILE__, __LINE__); \
      return Res::Err(rc, MSG);                   \
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

// #define Void (void())

template <typename T = void>
class Result
{
private:
  std::optional<uint32_t> code;
  std::optional<std::string> desc;
  std::optional<T> _value;

public:
  Result(const Result<T> &other)
  {
    code = other.code;
    desc = other.desc;
    _value = other._value;
  }

  Result() {}

  static Result<T> Ok(T value)
  {
    Result<T> res;
    res._value = value;
    return res;
  }
  static Result<T> Err(uint32_t code, std::string desc)
  {
    Result<T> res;
    res.code = code;
    res.desc = desc;
    return res;
  }

  T value() { return _value.value(); }
  bool is_ok() { return _value.has_value(); }
  bool is_err() { return !_value.has_value(); }
  uint32_t rc()
  {
    if (code)
      return code.value();
    else
      return 0;
  }
  std::string msg()
  {
    if (desc)
    {
      return std::string("NO error");
    }
    else
    {
      return desc.value();
    }
  }
  template <typename U>
  Result<U> map(std::function<Result<U>(T)> f)
  {
    if (_value.has_value)
    {
      return Result<U>::Ok(f(_value.value()));
    }
    else
    {
      return Result<U>::Err(code.value(), desc.value());
    }
  }
};

class Res
{
private:
  uint32_t _rc;
  std::optional<std::string> _msg;

public:
  /*Res(const Res &other)
  {
    code = other.code;
    desc = other.desc;
  }
  constexpr Res& Res::operator=(const Res& other) {
    Res ret;
    ret.code = other.code;
    ret.desc = other.desc;
    return ret;
  }*/

  Res() { _rc = 0; }

  static Res Ok()
  {
    Res res;
    res._rc = 0;
    return res;
  }
  static Res Err(uint32_t code, std::string desc)
  {
    Res res;
    res._rc = code;
    res._msg = desc;
    return res;
  }

  bool is_ok() { return _rc == 0; }
  bool is_err() { return _rc != 0; }
  uint32_t rc() { return _rc; }
  std::string msg()
  {
    if (_rc==0)
    {
      return std::string("NO error");
    }
    else
    {
      return _msg.value();
    }
  }
  Res on_err(std::function<Res(uint32_t, std::string)> f)
  {
    if (is_err())
    {
      return f(_rc, _msg.value());
    }
    else
    {
      return Res::Ok();
    }
  }
  Res on_ok(std::function<Res()> f)
  {
    if (is_ok())
    {
      return f();
    }
    else
    {
      return Res::Err(_rc, _msg.value());
    }
  }
};

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

#endif
