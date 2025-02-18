# ON HOLD
# trying no_std on ESP32 
- I went back to C++ on ESP32 as there are just too many lacking libraries :
-- Zenoh-pico has no Rust wrapper
-- ESPIDF mcpwm has no complete Rust interface for capture
-- I have extended my C++ on ESP32 with some Rust concepts like Result<> 
## system setup
```
espup install
# install ESPI-IDF
sudo apt-get install git wget flex bison gperf python3 python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
mkdir -p ~/esp
cd ~/esp
git clone -b v5.1 --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh
```

## MQTT-SN protocol
https://groups.oasis-open.org/higherlogic/ws/public/download/66091/MQTT-SN_spec_v1.2.pdf
https://github.com/ty4tw/MQTT-SN

