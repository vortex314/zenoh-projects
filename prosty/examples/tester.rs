mod limero;
use limero::SysInfo;

fn main() -> anyhow::Result<()> {
    let sys_info = SysInfo {
        uptime: Some(123456),
        free_memory: None,
        total_memory: vec![4096, 8192],
        power_on: Some(limero::Toggle::On),
    };
    let serialized = serde_json::to_string(&sys_info)?;
    println!("Serialized SysInfo: {}", serialized);
    let deserialized: SysInfo = serde_json::from_str(&serialized)?;
    println!("Deserialized SysInfo: {:?}", deserialized);
    Ok(())
}