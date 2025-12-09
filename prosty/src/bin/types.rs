use pest_derive::Parser;

#[derive(Parser)]
#[grammar = "syntax/state_diagram.pest"]
pub struct StateDiagramParser;


#[derive(Debug, Clone)]
pub enum StateType {
    Normal,
    Choice,
    Fork,
    Join,
    Note,
    Divider,
}

#[derive(Debug, Clone)]
pub struct State {
    pub id: String,
    pub state_type: StateType,
    pub name: Option<String>,
    pub description: Option<String>,
    pub states: Vec<State>,
    pub transitions: Vec<Transition>,
    pub notes: Vec<Note>,
}

#[derive(Debug, Clone)]
pub struct Transition {
    pub from: StateReference,
    pub to: StateReference,
    pub operator: TransitionOperator,
    pub label: Option<String>,
    pub properties: Vec<Property>,
}

#[derive(Debug, Clone)]
pub enum StateReference {
    State(String),
    Initial,
    Final,
    WithSuffix { state: String, suffix: StateSuffix },
}

#[derive(Debug, Clone)]
pub enum StateSuffix {
    Entry,
    Exit,
    Do,
}

#[derive(Debug, Clone)]
pub enum TransitionOperator {
    Simple,
    Thick,
    Dotted,
    Length(usize),
    Styled(String),
}

#[derive(Debug, Clone)]
pub struct Property {
    pub name: String,
    pub value: PropertyValue,
}

#[derive(Debug, Clone)]
pub enum PropertyValue {
    String(String),
    Number(f64),
    Boolean(bool),
    Identifier(String),
}

#[derive(Debug, Clone)]
pub struct Note {
    pub position: NotePosition,
    pub state: StateReference,
    pub content: String,
}

#[derive(Debug, Clone)]
pub enum NotePosition {
    Right,
    Left,
    Top,
    Bottom,
    Default,
}

#[derive(Debug, Clone)]
pub struct Config {
    pub directives: Vec<(String, PropertyValue)>,
}