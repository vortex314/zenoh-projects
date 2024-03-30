#!/bin/bash

set -o errexit
set -o nounset
set -o pipefail
set -o xtrace

readonly TARGET_ARCH=armv7-unknown-linux-gnueabihf
# readonly TARGET_ARCH=armv7-unknown-linux-musleabihf
# readonly TARGET_ARCH=arm-unknown-linux-musleabi
readonly SOURCE_PATH=./target/${TARGET_ARCH}/release/serial_proxy

# cargo build --release --target=${TARGET_ARCH}
cross build --release --target=${TARGET_ARCH}
rcp ${SOURCE_PATH} pi1.local:.
rcp ${SOURCE_PATH} pi3.local:.
