#ifndef CHANNEL_H
#define CHANNEL_H
#include <freertos/FreeRTOS.h>
#include <freertos/mpu_wrappers.h>
#include <freertos/portmacro.h>
#include <freertos/queue.h>
#include <freertos/timers.h>
#include <stddef.h>
#include <vector>
#include <esp_timer.h>
#include <log.h>

template <typename T> class Channel {
public:
  Channel(size_t depth) { queue = xQueueCreate(depth, sizeof(T)); 
  INFO("Channel created [%d][%d]  ", depth, sizeof(T));
  }

  bool send(const T message, TickType_t timeout = portMAX_DELAY) {
    return xQueueSend(queue, &message, timeout) == pdTRUE;
  }

  bool receive(T& message, TickType_t timeout = portMAX_DELAY) {
    return xQueueReceive(queue, &message, timeout) == pdTRUE;
  }

  ~Channel() { vQueueDelete(queue); }

  QueueHandle_t getQueue() { return queue; }

private:
  QueueHandle_t queue;
};

// Queue Set wrapper
class QueueSet {
public:
  QueueSet(size_t maxQueues) { set = xQueueCreateSet(maxQueues); }

  void addQueue(QueueHandle_t queue) { xQueueAddToSet(queue, set); }

  QueueHandle_t wait(TickType_t timeout = portMAX_DELAY) {
    return xQueueSelectFromSet(set, timeout);
  }

  ~QueueSet() { vQueueDelete(set); }

private:
  QueueSetHandle_t set;
};

struct Timer {
  bool _auto_reload;
  uint64_t _expires_at;
  uint64_t _period;
  bool _active = false;
  int _id;

public:
  Timer(int id, bool auto_reload, bool active, uint64_t period,
        uint64_t expires_at) {
    _auto_reload = auto_reload;
    _active = active;
    _period = period;
    _expires_at = expires_at;
    _id = id;
  }

  static uint64_t now() { return esp_timer_get_time()/1000; }

  static Timer Repetitive(int id, uint64_t period) {
    return Timer(id, true, true, period, now() + period);
  }

  static Timer OneShot(int id, uint64_t delay) {
    return Timer(id, false, true, 0, now() + delay);
  }

  bool is_expired(uint64_t now) const & { return now >= _expires_at; }

  void update(uint64_t now) {
    if ( _active ) {
      if ( _auto_reload && _period > 0 ) {
        _expires_at = now + _period;
      } else {
        if ( now > _expires_at ) {
          _active = false;
        }
      }
    }
  }
  void start() {
    _active = true;
    _expires_at = now() + _period;
  }

  void stop() { _active = false; }

  void reset() { _expires_at = now() + _period; }

  int id() const & { return _id; }
};

class Timers {
public:
  std::vector<Timer> timers;
  Timers() {}
  void add_timer(Timer timer) { timers.push_back(timer); }
  void remove_timer(int id) {
    for (int i = 0; i < timers.size(); i++) {
      if (timers[i].id() == id) {
        timers.erase(timers.begin() + i);
        break;
      }
    }
  }

  uint64_t get_next_expires_at() {
    uint64_t expires_at = UINT64_MAX;
    for (const Timer& timer : timers) {
      if (timer._active && timer._expires_at < expires_at) {
        expires_at = timer._expires_at;
      }
    }
    return expires_at;
  }

  uint64_t sleep_time() {
    uint64_t expires_at = get_next_expires_at();
    uint64_t now = Timer::now();
    if ( expires_at <= now ) {
      return 0;
    }
    return expires_at - now;
  }

  std::vector<int> get_expired_timers() {
    uint64_t now = Timer::now();
    std::vector<int> expired_timers;
    for (const Timer &timer : timers) {
      if (timer.is_expired(now)) {
        expired_timers.push_back(timer.id());
      }
    }
    return expired_timers;
  }

  void update() {
    uint64_t now = Timer::now();
    for (Timer &timer : timers) {
      timer.update(now);
    }
  }
};

#endif