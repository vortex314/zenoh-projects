Got it—if **pkg-config** isn’t available and you only have the GitHub source of **zenoh‑pico**, you can still link cleanly by **building the C library yourself** and pointing the Rust crate to it via environment variables (e.g., `ZENOHPICO_DIR`) or by adding zenoh‑pico as an **ESP‑IDF component**. Below are two proven routes, with a small tweak to `build.rs`.

> References confirming build and integration options:  
> • The **zenoh‑pico** GitHub README shows how to build the library (CMake/Make) and how it’s integrated across platforms (Zephyr/Arduino/ESP‑IDF/Pico, etc.) and provides the dev headers you need. [1](blob:https://outlook.office.com/77c58447-0f49-4db0-a4da-bfc7a386f627)  
> • The **zenoh‑pico docs** detail configuration surfaces (mode, listen/connect, closures, slices/bytes) that we use from Rust via FFI. [\[deepwiki.com\]](https://deepwiki.com/eclipse-zenoh/zenoh-c/3-c-api-reference)

***

## Option A — Build zenoh‑pico and link from Rust via `ZENOHPICO_DIR`

1.  **Clone & build the library (on your host):**

```bash
git clone https://github.com/eclipse-zenoh/zenoh-pico.git
cd zenoh-pico
# minimal build:
make
sudo make install   # optional; if you don't install, just note build paths
```

This produces `libzenohpico.*` and installs headers under `include/` (or leaves them in the repo if you skip install). The README documents this flow for Unix builds and for embedded targets (ESP‑IDF, Pico, etc.). [1](blob:https://outlook.office.com/77c58447-0f49-4db0-a4da-bfc7a386f627)

2.  **Point the Rust build to the library and headers:**
    Set an env var that our crate already supports:

```bash
export ZENOHPICO_DIR=/absolute/path/to/zenoh-pico     # e.g. the repo root or your install prefix
# ensure it contains:
#  $ZENOHPICO_DIR/include/zenoh-pico/...
#  $ZENOHPICO_DIR/lib/libzenohpico.a (or .so)
```

3.  **Use the crate you downloaded** ([zenoh\_pico\_rs.zip](https://deepwiki.com/eclipse-zenoh/zenoh-pico/2-core-api)) and build:

```bash
cd zenoh_pico_rs
cargo build --release
```

### If your library is **static** (`libzenohpico.a`)

The `build.rs` already prints:

```rust
println!("cargo:rustc-link-lib=static=zenohpico");
println!("cargo:rustc-link-search=native={}/lib", dir);
println!("cargo:include={}/include", dir);
```

…and bindgen includes `wrapper.h` which has:

```c
#include <zenoh-pico/zenoh-pico.h>
```

This expects headers under `$ZENOHPICO_DIR/include/zenoh-pico/`. The layout follows the dev headers shipped with the repo. [1](blob:https://outlook.office.com/77c58447-0f49-4db0-a4da-bfc7a386f627)

### If your library is **shared** (`libzenohpico.so`)

Change one line in `build.rs`:

```rust
println!("cargo:rustc-link-lib=zenohpico"); // instead of static=
```

And ensure runtime loader can find it (e.g., `LD_LIBRARY_PATH=$ZENOHPICO_DIR/lib` on Linux).

***

## Option B — Add zenoh‑pico as an **ESP‑IDF component** and link from there

This approach keeps the C library compiled with ESP toolchain and then points Rust to the result.

1.  **Add zenoh‑pico to your ESP‑IDF project**:

*   Create `components/zenoh-pico/` in your ESP‑IDF app and copy `src/` + `include/` + minimal `CMakeLists.txt` for the component; or add zenoh‑pico as a submodule and reference it from `CMakeLists.txt`.
*   Build your ESP app once; ESP‑IDF will compile zenoh‑pico with the Xtensa/ESP toolchain producing `libzenohpico.a`.

2.  **Expose the paths to your Rust crate**:
    Set:

```bash
export ZENOHPICO_DIR=/absolute/path/to/your/esp-idf-app/build
# Or to a staging directory that contains:
# include/zenoh-pico/... and lib/libzenohpico.a produced by ESP-IDF
```

If you keep headers inside the component path, you can export a different include dir:

```bash
export ZENOHPICO_INCLUDE=/absolute/path/to/your/esp-idf-app/components/zenoh-pico/include
export ZENOHPICO_LIB=/absolute/path/to/your/esp-idf-app/build/<component>/libzenohpico.a
```

Then adjust `build.rs` minimally:

```rust
if let (Ok(inc), Ok(libdir)) = (env::var("ZENOHPICO_INCLUDE"), env::var("ZENOHPICO_LIB")) {
    println!("cargo:rustc-link-search=native={}", std::path::Path::new(&libdir).parent().unwrap().display());
    println!("cargo:rustc-link-lib=static=zenohpico");
    println!("cargo:include={}", inc);
    // Also pass the include to bindgen
    builder = builder.clang_arg(format!("-I{}", inc));
} else if let Ok(dir) = env::var("ZENOHPICO_DIR") {
    // fallback same as Option A
}
```

> The GitHub repo documents ESP‑IDF flow and PlatformIO variants; you essentially compile zenoh‑pico with the ESP toolchain (so the ABI matches) and just point Rust to the resulting `.a` + headers. [1](blob:https://outlook.office.com/77c58447-0f49-4db0-a4da-bfc7a386f627)

***

## Why we avoided pkg‑config

While the repo ships a `zenohpico.pc` in its tree, embedded toolchains (ESP‑IDF) often don’t use host pkg‑config easily. Building and linking by **explicit paths** is reliable and portable across your CI/toolchains. The **ReadTheDocs** API details don’t depend on pkg‑config; they only define the expected C symbols we bind to. [1](blob:https://outlook.office.com/77c58447-0f49-4db0-a4da-bfc7a386f627) [\[deepwiki.com\]](https://deepwiki.com/eclipse-zenoh/zenoh-c/3-c-api-reference)

***

## ESP‑IDF multicast peer mode reminder

*   Use **peer mode** + **UDP multicast endpoints** (e.g., `udp/224.0.0.224:7447`), matching your network interface constraints.
*   Keep `Session::poll()` in a FreeRTOS loop/task to drive RX/TX.
*   Ensure the **batch size** is consistent across nodes (zenoh‑pico warns defaults differ per platform); if needed, build zenoh‑pico with adjusted `BATCH_MULTICAST_SIZE`/`FRAG_MAX_SIZE`. [1](blob:https://outlook.office.com/77c58447-0f49-4db0-a4da-bfc7a386f627)

***

## TL;DR

*   **No pkg‑config?** No problem: build **zenoh‑pico** from GitHub, then set `ZENOHPICO_DIR` (or `ZENOHPICO_INCLUDE` + `ZENOHPICO_LIB`) so `build.rs` can find headers/libs. [1](blob:https://outlook.office.com/77c58447-0f49-4db0-a4da-bfc7a386f627)
*   The Rust crate you downloaded already supports this workflow; you only need to point it to the C artifacts.

If you’d like, I can:

*   **Edit `build.rs`** in your zip to support `ZENOHPICO_INCLUDE`/`ZENOHPICO_LIB` explicitly,
*   Provide a **minimal ESP‑IDF component `CMakeLists.txt`** for zenoh‑pico,
*   Or craft a **README** with step‑by‑step ESP‑IDF + Rust build instructions.

Which one would help you most right now?
