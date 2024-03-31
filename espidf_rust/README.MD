
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

