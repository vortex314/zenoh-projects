fn main() {
    println!("cargo:rerun-if-changed=src/message.proto");
    println!("Running build.rs to generate protobuf code...");
    prost_build::compile_protos(&["src/message.proto"], &["src/"]).unwrap();
}
