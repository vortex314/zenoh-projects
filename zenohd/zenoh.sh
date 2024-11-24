cargo build --bin zenohd  --features transport_tcp --features transport_serial --release --all-targets -j2
# cargo build --bin zenohd  --no-default-features --features transport_tcp --features transport_serial --release --all-targets -j2
# cargo build --bin zenohd --example z_sub --no-default-features --features transport_tcp --features transport_serial --features shared-memory --release --all-targets -j1
# export RUST_LOG="zenoh_transport::unicast::universal::rx=trace,transport=trace"
# export RUST_LOG="zenoh_transport=trace,z_serial=trace,zenoh_links=trace,zenoh_link_serial=trace"
# export RUST_LOG=trace
export RUST_BACKTRACE=1
# strace -e trace=open,read,write -o strace.log ./target/debug/zenohd --config=./zenoh.json5
# strace -t -f -o strace3.log ./target/debug/zenohd --config=./zenoh.json5
./target/release/zenohd --config=./zenoh.json5
