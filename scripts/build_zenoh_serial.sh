cd ../../zenoh
cargo build -j 3 --release --all-targets --no-default-features --features transport_tcp --features transport_serial

