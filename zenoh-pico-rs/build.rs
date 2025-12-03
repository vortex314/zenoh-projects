// build.rs
use std::env;   
use std::path::PathBuf;
use glob::glob;

fn main() {
    let library_path = "/home/lieven/workspace/zenoh-projects/zenoh-esp32-eventbus/.pio/build/hoverboard/lib44a";
    println!("cargo:rustc-link-search=native={}", library_path);
    println!("cargo:rustc-link-lib=static=zenoh-pico");
    println!("cargo:rustc-link-arg-bins=-Tlinkall.x");
    println!("cargo:rustc-link-arg-bins=-Trom_functions.x");
}