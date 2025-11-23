

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

