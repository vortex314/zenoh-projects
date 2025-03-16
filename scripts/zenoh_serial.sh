RUST_LOG=debug ../../zenoh/target/release/zenohd --no-multicast-scouting -l "serial//dev/ttyUSB0#baudrate=115200" -l “tcp/127.0.0.1:7447”

