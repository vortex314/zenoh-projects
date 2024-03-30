#!/usr/bin/env bash
rm -rf rust
cd ../../cddl-codegen/
target/debug/cddl-codegen --input=../zenoh-projects/proxy_rs/protocol.cddl --output=../zenoh-projects/proxy_rs 
# apt-get install libudev-dev libssl-dev -y