# cargo build --bin zenohd --example z_sub --no-default-features --features transport_tcp --features transport_serial
# export RUST_LOG="zenoh_transport::unicast::universal::rx=trace,transport=trace"
 export RUST_LOG="zenoh_transport::unicast=trace,z_serial=trace"
# export RUST_LOG=trace
~/workspace/zenoh/target/debug/zenohd --config=./zenoh.json5
