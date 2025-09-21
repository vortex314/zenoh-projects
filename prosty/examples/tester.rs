mod limero;
use limero::SysInfo;

fn main() -> anyhow::Result<()> {
    let sys_info = SysInfo {
        uptime: Some(123456),
        free_heap: None,
        flash: Some(204800),
        cpu_board: Some("ESP32".to_string()),
    };
    let serialized = serde_json::to_string(&sys_info)?;
    println!("Serialized SysInfo: {}", serialized);
    let deserialized: SysInfo = serde_json::from_str(&serialized)?;
    println!("Deserialized SysInfo: {:?}", deserialized);
    Ok(())
}