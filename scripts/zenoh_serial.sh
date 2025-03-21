set -x
RUST_LOG=trace ../../zenoh/target/release/zenohd --no-multicast-scouting -l "serial//dev/ttyUSB0#baudrate=115200" -l "tcp/127.0.0.1:7447" --cfg='transport/unicast/open_timeout:1000' --cfg='transport/unicast/accept_timeout:100000000'

# RUST_LOG=trace ../../zenoh/target/release/zenohd --no-multicast-scouting -l "serial//dev/ttyUSB0#baudrate=115200" --cfg='transport/unicast/open_timeout:1000' --cfg='transport/unicast/accept_timeout:100000000'

