
# Base image from cross-rs for aarch64 glibc
FROM ghcr.io/cross-rs/aarch64-unknown-linux-gnu:latest

# Install libudev development files and pkg-config inside the container.
# The base image is Debian/Ubuntu-like; no need to enable multiarch here.
RUN dpkg --add-architecture arm64
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        libudev-dev:arm64 \
        pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Optional: ensure pkg-config can find libudev.pc (usually already on default path)
# Ensure pkg-config can find ARM64 .pc files
ENV PKG_CONFIG_ALLOW_SYSTEM_LIBS=1 \    
	PKG_CONFIG_ALLOW_SYSTEM_CFLAGS=1 \    
	PKG_CONFIG_PATH=/usr/lib/aarch64-linux-gnu/pkgconfig

# Sanity check (executed at build time; harmless if skipped)
# You can uncomment to verify during docker build:
RUN pkg-config --cflags --libs libudev && ldconfig -p | grep libudev
