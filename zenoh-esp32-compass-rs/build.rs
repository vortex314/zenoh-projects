// Add to end of build.rs
use std::env;
use std::path::PathBuf;

fn main() {
    // ... (previous zenoh-pico build code)

    // Generate bindings
    let bindings = bindgen::Builder::default()
        .header("wrapper.h")  // We'll create this next
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        .generate()
        .expect("Failed to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("zenoh_bindings.rs"))
        .expect("Couldn't write bindings!");
}