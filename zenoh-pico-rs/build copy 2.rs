// build.rs
use std::env;   
use std::path::PathBuf;
use glob::glob;

fn main() {
    let idf_path = env::var("IDF_PATH").unwrap_or("/home/lieven/esp/esp-idf".to_string());
    let zenoh_pico_root = "/home/lieven/workspace/zenoh-pico";
    let vendor_path = PathBuf::from(&zenoh_pico_root);
    let src_path = vendor_path.join("src");
    let include_path = vendor_path.join("include");

    // 1. Gather all .c files using glob
    // Collect all C files from specific directories
    let glob_patterns = vec![
        "api/**/*.c",
        "collections/**/*.c",
        "link/**/*.c",
        "net/**/*.c",
        "protocol/**/*.c",
        "session/**/*.c",
        "transport/**/*.c",
        "utils/**/*.c",
        "system/espidf/**/*.c",
        "system/common/**/*.c",
    ];
    
    let mut c_files = Vec::new();
    for pattern in glob_patterns {
        let full_pattern = PathBuf::from(&src_path).join(pattern);
        for entry in glob(full_pattern.to_str().unwrap()).expect("Failed to read glob pattern") {
            match entry {
                Ok(path) => {
                    println!("cargo:rerun-if-changed={}", path.display());
                    c_files.push(path);
                }
                Err(e) => println!("cargo:warning=Error reading path: {:?}", e),
            }
        }
    }
    
    // Note: Zenoh-pico often amalgamates sources, but if not, 
    // you might need to glob "vendor/src/**/*.c"

    // 2. Configure the Compiler (cc)
    let mut build = cc::Build::new();
    
    build
        .include(&include_path)
        .warnings(false); // Zenoh-pico might have warnings that clutter output
    
    // Add ESP-IDF include paths if building for ESP-IDF target
    let target = env::var("TARGET").unwrap();
    if target.contains("espidf") {
        // Get ESP-IDF paths from environment
            let idf_components = PathBuf::from(&idf_path).join("components");
            
            // Add common ESP-IDF component includes
            build.include(idf_components.join("freertos/FreeRTOS-Kernel/include"));
            build.include(idf_components.join("freertos/esp_additions/include"));
            build.include(idf_components.join("esp_hw_support/include"));
            build.include(idf_components.join("esp_system/include"));
            build.include(idf_components.join("esp_common/include"));
            build.include(idf_components.join("lwip/lwip/src/include"));
            build.include(idf_components.join("lwip/port/include"));
            build.include(idf_components.join("driver/include"));
            build.include(idf_components.join("soc/include"));
            build.include(idf_components.join("hal/include"));
            build.include(idf_components.join("esp_driver_uart/include"));
            // components/nvs_flash/test_nvs_host/
            build.include(idf_components.join("nvs_flash/test_nvs_host"));
            build.include(idf_components.join("soc/esp32/include/"));
            build.include(idf_components.join("freertos/config/include/freertos/"));
            build.include(idf_components.join("freertos/config/xtensa/include"));
            build.include(idf_components.join("xtensa/include"));
            build.include(idf_components.join("xtensa/esp32/include/"));
            build.include(idf_components.join("freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos/"));
            build.include(idf_components.join("newlib/platform_include"));
            build.include(idf_components.join("heap/include"));
            build.include(idf_components.join("esp_rom/include"));
            
            // Add build config directory if available
            if let Ok(build_path) = env::var("ESP_IDF_SDKCONFIG_DEFAULTS") {
                if let Some(config_dir) = PathBuf::from(build_path).parent() {
                    build.include(config_dir);
                }
            }
        
        // --- CONFIGURATION FLAGS for ESP-IDF ---
        build.define("ZENOH_ESPIDF", "1"); // Targeting ESP-IDF
    }
    
    build
        .define("ZENOH_PICO_WITH_TCP", None)
        .define("ZENOH_PICO_WITH_UDP", None)
        .define("ZENOH_PICO_WITH_SERIAL_TRANSPORT", "0") 
        .define("ZENOH_ESPIDF", "1") // Targeting ESP-IDF
        // If on ESP32, enable this to avoid thread issues if not using pthreads
        .define("ZENOH_PICO_NO_THREAD", None); 

    // Add all collected C files
    for file in &c_files {
        build.file(file);
    }

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