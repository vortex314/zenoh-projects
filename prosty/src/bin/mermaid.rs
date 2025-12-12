// https://docs.mermaidchart.com/mermaid-oss/syntax/stateDiagram.html


mod parser;
mod types;
use crate::parser::MermaidStateDiagram;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let input = r#"
```mermaid
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
    
    println!("Parsed {} states", diagram.states.len());
    println!("Parsed {} transitions", diagram.transitions.len());
    
    Ok(())
}