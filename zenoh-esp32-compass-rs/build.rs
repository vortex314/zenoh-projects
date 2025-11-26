fn main() {
    // Tell Cargo to re-run this build script if these files change
    println!("cargo:rerun-if-changed=build.rs");
    
    // For now, zenoh-pico bindings are disabled until zenoh-pico is properly
    // set up as an ESP-IDF component. To enable:
    // 1. Add zenoh-pico as an ESP-IDF component in the components/ directory
    // 2. Configure the include paths for bindgen
    // 3. Uncomment the binding generation code below
    
    /*
    use std::env;
    use std::path::PathBuf;
    
    // Generate bindings
    // For ESP32 cross-compilation, we need to use clang args that work with the host
    // rather than the xtensa target which clang doesn't understand
    let bindings = bindgen::Builder::default()
        .header("wrapper.h")
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new()))
        // Use host target for clang, not the xtensa target
        .clang_arg("-target")
        .clang_arg(env::var("HOST").unwrap_or_else(|_| "x86_64-unknown-linux-gnu".to_string()))
        // Add include path for zenoh-pico headers
        // .clang_arg("-I/path/to/zenoh-pico/include")
        .generate()
        .expect("Failed to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("zenoh_bindings.rs"))
        .expect("Couldn't write bindings!");
    */
}
