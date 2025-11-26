# ../../zenoh/target/release/examples/z_sub -k "src/**" -l udp/224.0.0.123:7447 --no-multicast-scouting 
# ../../zenoh/target/release/examples/z_pub -k "dst/esp1/sys/SysCmd/JSON" -e tcp/127.0.0.1:7447 --no-multicast-scouting  -p "{\"console\":\"================> from linux\"}"
../../zenoh/target/release/examples/z_pub -k "dst/brain/ps4/Ps4Cmd" \
    -e tcp/127.0.0.1:7447 \
    -p "{\"led_red\":0,\"led_green\":127,\"led_blue\":0,\"led_flash_on\":127,\"led_flash_off\":127}"
