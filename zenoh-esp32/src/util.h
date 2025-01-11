#ifndef UTIL_H
#define UTIL_H

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>
#include <log.h>

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

#define RET_ERRI(VAL, MSG)                       \
  {                                              \
    int rc = (VAL);                              \
    if (rc < 0)                                  \
    {                                            \
      INFO(MSG);                                 \
      INFO("=%d %s:%d", rc, __FILE__, __LINE__); \
      return Res::Err(rc, MSG);                  \
    }                                            \
  }

  #define CHECK(VAL)                       \
  {                                              \
    int rc = (VAL);                              \
    if (rc < 0)                                  \
    {                                            \
      ERROR("=%d %s:%d %s", rc, __FILE__, __LINE__,#VAL ); \
      return Res::Err(rc, #VAL);                  \
    }                                            \
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
  uint32_t code;
  std::optional<std::string> desc;

public:
  Res(const Res &other)
  {
    code = other.code;
    desc = other.desc;
  }

  Res() { code = 0; }

  static Res Ok()
  {
    Res res;
    res.code = 0;
    return res;
  }
  static Res Err(uint32_t code, std::string desc)
  {
    Res res;
    res.code = code;
    res.desc = desc;
    return res;
  }

  bool is_ok() { return code == 0; }
  bool is_err() { return code != 0; }
  uint32_t rc() { return code; }
  std::string msg()
  {
    if (code)
    {
      return std::string("NO error");
    }
    else
    {
      return desc.value();
    }
  }
  Res on_err(std::function<Res(uint32_t, std::string)> f)
  {
    if (is_err())
    {
      return f(code, desc.value());
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
      return Res::Err(code, desc.value());
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
#endif
