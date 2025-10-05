set -x
# RUST_LOG=trace
export RUST_BACKTRACE=1
echo $RUST_BACKTRACE
echo $PWD
RUST_LOG=info zenohd --no-multicast-scouting \
    -l "udp/224.0.0.123:7447" \
    -l "tcp/127.0.0.1:7447" \
    --rest-http-port=8000 \
    --cfg='transport/link/tx/batch_size:2048' \
    -c $PWD/config.json5

#    --cfg='transport/unicast/open_timeout:1000' \
#    --cfg='transport/unicast/accept_timeout:100000000' \
#    -l "serial//dev/ttyUSB0#baudrate=115200" \
#    -l "serial//dev/ttyUSB1#baudrate=921600" \

# RUST_LOG=info ../../zenoh/target/release/zenohd --no-multicast-scouting -l "serial//dev/ttyUSB0#baudrate=115200" --cfg='transport/unicast/open_timeout:1000' --cfg='transport/unicast/accept_timeout:100000000'

