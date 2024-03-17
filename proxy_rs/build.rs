use std::process::Command;

fn main() -> std::io::Result<()> {
    Command::new("../../cddl-codegen/target/debug/cddl-codegen")
        .arg("--input=protocol.cddl")
        .arg("--output=src/protocol.rs")
        .output()
        .expect( "failed to execute process");
    println!("cargo:rerun-if-changed=protocol.cddl");
    Ok(())

}