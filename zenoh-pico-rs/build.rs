// build.rs
use std::env;
use std::path::PathBuf;

fn main() {
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let vendor_path = PathBuf::from(&manifest_dir).join("vendor");
    let src_path = vendor_path.join("src");
    let include_path = vendor_path.join("include");

    // 1. Gather all .c files
    let c_files = [
        "src/zenoh-pico.c", 
        "src/transport/transport.c",
        // Add other source files explicitly or use a glob pattern if you prefer
        // Usually zenoh-pico.c and transport.c handle the bulk, 
        // but check the /src folder for others you might need like 'system.c'
    ];
    
    // Note: Zenoh-pico often amalgamates sources, but if not, 
    // you might need to glob "vendor/src/**/*.c"

    // 2. Configure the Compiler (cc)
    let mut build = cc::Build::new();
    
    build
        .include(&include_path)
        .warnings(false) // Zenoh-pico might have warnings that clutter output
        
        // --- CONFIGURATION FLAGS ---
        .define("ZENOH_PICO_WITH_TCP", None)
        .define("ZENOH_PICO_WITH_UDP", None)
        .define("ZENOH_PICO_WITH_SERIAL_TRANSPORT", "0") 
        // If on ESP32, enable this to avoid thread issues if not using pthreads
        .define("ZENOH_PICO_NO_THREAD", None); 

    // Add all sources
    // Note: If zenoh-pico structure changes, you might need to use glob
    build.file(src_path.join("zenoh-pico.c"));
    // Add other necessary C files here if they aren't included by zenoh-pico.c

    // 3. Compile the static library
    build.compile("zenohpico"); // Creates libzenohpico.a

    // 4. Generate Bindings (Bindgen)
    // This reads the C header and creates Rust structs/functions
    let bindings = bindgen::Builder::default()
        .header(include_path.join("zenoh-pico.h").to_string_lossy())
        .parse_callbacks(Box::new(bindgen::CargoCallbacks::new())) // Invalidate crate when C changes
        .generate()
        .expect("Unable to generate bindings");

    // Write the bindings to the $OUT_DIR/bindings.rs
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}