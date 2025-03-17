cd ../../zenoh
git checkout 1.2.1
cargo build --release --all-targets --no-default-features --features transport_tcp,transport_serial
cargo build -j 3 --release --all-targets --no-default-features --features transport_tcp,transport_serial

