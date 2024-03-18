use std::process::Command;

fn main() -> std::io::Result<()> {
    let result = Command::new("../../cddl-codegen/target/debug/cddl-codegen")
        .arg("--input=protocol.cddl")
        .arg("--output=src/protocol.rs")
        .output();
    println!("cargo:rerun-if-changed=protocol.cddl");
    match result {
        Ok(output) => {
            if !output.status.success() {
                println!("cargo:warning=cddl-codegen failed");
                println!("cargo:warning=stdout: {}", String::from_utf8_lossy(&output.stdout));
                println!("cargo:warning=stderr: {}", String::from_utf8_lossy(&output.stderr));
            }
        }
        Err(err) => {
            println!("cargo:warning=cddl-codegen failed to start: {}", err);
        }
    }
    Ok(())
}