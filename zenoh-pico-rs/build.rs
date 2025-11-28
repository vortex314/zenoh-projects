use std::env;
use std::path::PathBuf;

fn main() {
    if pkg_config::probe_library("zenohpico").is_err() {
        if let Ok(dir) = env::var("ZENOHPICO_DIR") {
            println!("cargo:rustc-link-search=native={}/lib", dir);
            println!("cargo:rustc-link-lib=static=zenohpico");
            println!("cargo:include={}/include", dir);
        } else {
            panic!("Could not find libzenohpico");
        }
    }

    let bindings = bindgen::Builder::default()
        .header("wrapper.h")
        .allowlist_type("z_.*|zp_.*")
        .allowlist_function("z_.*|zp_.*")
        .allowlist_var("Z_.*|ZP_.*")
        .generate()
        .expect("Unable to generate bindings");

    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings.write_to_file(out_path.join("bindings.rs")).unwrap();
    println!("cargo:rerun-if-changed=wrapper.h");
}
