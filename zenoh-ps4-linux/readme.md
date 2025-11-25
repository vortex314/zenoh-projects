

# Build instructions
## Build docker image with libudev for arm64 based on Dockerfile
```sh
docker build -t cross-aarch64-libudev .
```
## Build arm64 image of executable
```sh
cross build --target aarch64-unknown-linux-gnu
```
## Copy to remote host pi.local
```sh
scp target/aarch64-unknown-linux-gnu/debug/zenoh-ps4-linux pi.local:.
ssh pi.local:./zenoh-ps-linux
```

```sh
sudo apt install libevdev2 joystick
```

# Appendix
## Cross compilation
```sh
docker run -it --rm ghcr.io/cross-rs/aarch64-unknown-linux-gnu:latest
apt-get install -y libudev-dev libevdev-dev pkg-config
apt-get install -y libudev-dev:arm64 libevdev-dev:arm64 pkg-config:arm64
```
# Test with bluer

sudo /usr/sbin/bluetoothd --compat

The --compat flag forces BlueZ into legacy mode, which skips the buggy headset negotiation and keeps the connection stable forever.


[bluetooth]# remove 8C:41:F2:D2:E5:48  # or whatever the current MAC is
[bluetooth]# scan on
# Put controller in pairing mode (Share + PS → double-flash white)
# Wait for [NEW] Device XX:XX:XX:XX:XX:XX Wireless Controller
[bluetooth]# pair XX:XX:XX:XX:XX:XX
[bluetooth]# trust XX:XX:XX:XX:XX:XX
[bluetooth]# connect XX:XX:XX:XX:XX:XX
[bluetooth]# exit
