// https://docs.mermaidchart.com/mermaid-oss/syntax/stateDiagram.html
mod logger;
use logger::init;
mod parser;
mod types;
use crate::parser::MermaidStateDiagram;
use log::info;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    init();
    let input = r#"```mermaid
stateDiagram-v2
    [*] --> Still
    Still --> [*]
    
    Still --> Moving : start moving
    Moving --> Still : stop moving
    Moving --> Crash : crash
    Crash --> [*]
    
    state Moving {
        Accelerating --> Cruising : speed >= 60
        Cruising --> Braking : obstacle detected
        Braking --> Accelerating : obstacle cleared
    }
    
    note right of Moving : This state\nhas internal transitions
```
    "#;
    
    let diagram = MermaidStateDiagram::parse(input)?;
    
    info!("Parsed {} states", diagram.states.len());
    info!("Parsed {} transitions", diagram.transitions.len());
    
    Ok(())
}