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


