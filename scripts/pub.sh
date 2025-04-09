# ../../zenoh/target/release/examples/z_sub -k "src/**" -l udp/224.0.0.123:7447 --no-multicast-scouting 
../../zenoh/target/release/examples/z_pub -k "dst/esp1/wifi" -e tcp/127.0.0.1:7447 --no-multicast-scouting 
