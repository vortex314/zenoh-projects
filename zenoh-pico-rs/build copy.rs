use std::env;
use std::path::PathBuf;

fn main() {
    let target = env::var("TARGET").unwrap();
    let manifest_dir = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    
    // For ESP-IDF builds, zenoh-pico is built as an ESP-IDF component
    // The linking is handled by esp-idf-sys through the CMakeLists.txt
    if !target.contains("espidf") {
        // For non ESP-IDF targets, try pkg-config or ZENOHPICO_DIR
        if pkg_config::probe_library("zenohpico").is_err() {
            if let Ok(dir) = env::var("ZENOHPICO_DIR") {
                println!("cargo:rustc-link-search=native={}/lib", dir);
                println!("cargo:rustc-link-lib=static=zenohpico");
            } else {
                panic!("Could not find libzenohpico. Either install it via pkg-config or set ZENOHPICO_DIR");
            }
        }
    } else {
        // For ESP-IDF, tell esp-idf-sys about our component
        println!("cargo:rustc-env=ESP_IDF_COMP_ZENOH_PICO=y");
    }

    // Prepare clang args for bindgen
    let mut clang_args = vec![];
    let mut include_path = None;
    
    // Add zenoh-pico include path
    if target.contains("espidf") {
        // For ESP-IDF, use the component in our workspace
        let component_include = manifest_dir.join("components/zenoh-pico/include");
        include_path = Some(component_include.clone());
        clang_args.push(format!("-I{}", component_include.display()));
        
        // For bindgen, use ZENOH_LINUX platform for simple type definitions
        // The actual ESP-IDF platform will be used at compile time by the C compiler
        clang_args.push("-DZENOH_LINUX".to_string());
        
        // Override target to x86_64 for bindgen since clang doesn't know xtensa
        clang_args.push("--target=x86_64-unknown-linux-gnu".to_string());
        
        // Use native host includes
        clang_args.push("-I/usr/lib/gcc/x86_64-linux-gnu/13/include".to_string());
        clang_args.push("-I/usr/include/x86_64-linux-gnu".to_string());
        clang_args.push("-I/usr/include".to_string());
    } else if let Ok(dir) = env::var("ZENOHPICO_DIR") {
        let inc_dir = PathBuf::from(&dir).join("include");
        include_path = Some(inc_dir.clone());
        clang_args.push(format!("-I{}", inc_dir.display()));
    }
    
    // Debug: print the include path
    if let Some(ref path) = include_path {
        eprintln!("Using zenoh-pico include path: {}", path.display());
    }

    let bindings = bindgen::Builder::default()
        .header("wrapper.h")
        .clang_args(clang_args)
        .allowlist_type("z_.*|zp_.*")
        .allowlist_function("z_.*|zp_.*")
        .allowlist_var("Z_.*|ZP_.*")
        .derive_default(true)
        // Use layout tests only for types that will have consistent size
        .layout_tests(false)
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings.write_to_file(out_path.join("bindings.rs")).unwrap();
    println!("cargo:rerun-if-changed=wrapper.h");
}
