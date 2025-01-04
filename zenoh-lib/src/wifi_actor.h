#include <channel.h>
#include <functional>
#include <serdes.h>
#include <vector>

typedef enum {
  WIFI_CONNECTED,
  WIFI_DISCONNECTED,
} WifiSignal;

struct WifiEvent {
  std::optional<WifiSignal> signal;
};

struct WifiCmd {
  std::optional<bool> stop_actor;
};

class WifiActor {
public:
  std::vector<std::function<void(WifiEvent)>> handlers;
  Channel<WifiCmd> cmds;
  Timers timers;

public:
  WifiActor();
  void run();
  void emit(WifiEvent event);
  void wifi_init_sta(void);
  static void event_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data);

private:
  esp_event_handler_instance_t handler_any_id;
  esp_event_handler_instance_t handler_got_ip;
  bool _wifi_connected = false;
};
