set -x
RUST_LOG=info ../../zenoh/target/release/zenohd --no-multicast-scouting \
    -l "udp/224.0.0.123:7447" \
    --cfg='transport/unicast/open_timeout:1000' \
    --cfg='transport/unicast/accept_timeout:100000000' \
    --cfg='transport/link/tx/batch_size:2048' \
    -l "tcp/127.0.0.1:7447" \

#    -l "serial//dev/ttyUSB0#baudrate=115200" \
#    -l "serial//dev/ttyUSB1#baudrate=921600" \

# RUST_LOG=info ../../zenoh/target/release/zenohd --no-multicast-scouting -l "serial//dev/ttyUSB0#baudrate=115200" --cfg='transport/unicast/open_timeout:1000' --cfg='transport/unicast/accept_timeout:100000000'

