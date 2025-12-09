use pest::Parser;
use pest::iterators::Pair;
use crate::types::{
    State, StateType, Transition, StateReference, TransitionOperator,
    Note, PropertyValue, Config,
};
use crate::types::StateDiagramParser;
use crate::types::Rule;

pub struct MermaidStateDiagram {
    pub config: Option<Config>,
    pub states: Vec<State>,
    pub transitions: Vec<Transition>,
    pub notes: Vec<Note>,
}

impl MermaidStateDiagram {
    pub fn parse(input: &str) -> Result<Self, Box<dyn std::error::Error>> {
        let pairs = StateDiagramParser::parse(Rule::stateDiagram, input)?;
        
        let mut diagram = MermaidStateDiagram {
            config: None,
            states: Vec::new(),
            transitions: Vec::new(),
            notes: Vec::new(),
        };
        
        for pair in pairs {
            match pair.as_rule() {
                /*Rule::config_directive => {
                    diagram.config = Some(Self::parse_config(pair));
                }*/
                Rule::state_declaration => {
                    diagram.states.push(Self::parse_state(pair));
                }
                Rule::transition => {
                    diagram.transitions.push(Self::parse_transition(pair));
                }
                Rule::note => {
                    diagram.notes.push(Self::parse_note(pair));
                }
                _ => {}
            }
        }
        
        Ok(diagram)
    }
    
    fn parse_config(pair: Pair<Rule>) -> Config {
        let mut config = Config { directives: Vec::new() };
        
        for inner_pair in pair.into_inner() {
            if inner_pair.as_rule() == Rule::key_value_pair {
                let mut kv_iter = inner_pair.into_inner();
                let key = kv_iter.next().unwrap().as_str().to_string();
                let value = Self::parse_value(kv_iter.next().unwrap());
                config.directives.push((key, value));
            }
        }
        
        config
    }
    
    fn parse_state(pair: Pair<Rule>) -> State {
        let mut state = State {
            id: String::new(),
            state_type: StateType::Normal,
            name: None,
            description: None,
            states: Vec::new(),
            transitions: Vec::new(),
            notes: Vec::new(),
        };
        
        for inner_pair in pair.into_inner() {
            match inner_pair.as_rule() {
                Rule::state_id => state.id = inner_pair.as_str().to_string(),
                Rule::state_type => {
                    state.state_type = match inner_pair.into_inner().next().unwrap().as_str() {
                        "choice" => StateType::Choice,
                        "fork" => StateType::Fork,
                        "join" => StateType::Join,
                        "note" => StateType::Note,
                        "divider" => StateType::Divider,
                        _ => StateType::Normal,
                    };
                }
                Rule::state_name => {
                    let content = Self::parse_string_content(inner_pair.into_inner().next().unwrap());
                    state.name = Some(content);
                }
                Rule::state_body => {
                    for body_item in inner_pair.into_inner() {
                        match body_item.as_rule() {
                            Rule::state_declaration => {
                                state.states.push(Self::parse_state(body_item));
                            }
                            Rule::transition => {
                                state.transitions.push(Self::parse_transition(body_item));
                            }
                            Rule::note => {
                                state.notes.push(Self::parse_note(body_item));
                            }
                            _ => {}
                        }
                    }
                }
                _ => {}
            }
        }
        
        state
    }
    
    fn parse_transition(pair: Pair<Rule>) -> Transition {
        let mut transition = Transition {
            from: StateReference::State(String::new()),
            to: StateReference::State(String::new()),
            operator: TransitionOperator::Simple,
            label: None,
            properties: Vec::new(),
        };
        
        let mut inner_pairs = pair.into_inner();
        
        // Parse 'from' state
        if let Some(from_pair) = inner_pairs.next() {
            transition.from = Self::parse_state_reference(from_pair);
        }
        
        // Parse operator
        if let Some(op_pair) = inner_pairs.next() {
            transition.operator = Self::parse_transition_operator(op_pair);
        }
        
        // Parse 'to' state
        if let Some(to_pair) = inner_pairs.next() {
            transition.to = Self::parse_state_reference(to_pair);
        }
        
        // Parse optional label
        if let Some(label_pair) = inner_pairs.next() {
            transition.label = Some(Self::parse_string_content(label_pair));
        }
        
        transition
    }
    
    fn parse_state_reference(pair: Pair<Rule>) -> StateReference {
        // Implementation for parsing state references
        // (handle initial [*], final [*], and state suffixes)
        todo!()
    }
    
    fn parse_transition_operator(pair: Pair<Rule>) -> TransitionOperator {
        // Implementation for parsing transition operators
        todo!()
    }
    
    fn parse_note(pair: Pair<Rule>) -> Note {
        // Implementation for parsing notes
        todo!()
    }
    
    fn parse_value(pair: Pair<Rule>) -> PropertyValue {
        match pair.as_rule() {
            Rule::string_literal => PropertyValue::String(Self::unquote(pair.as_str())),
            Rule::number => PropertyValue::Number(pair.as_str().parse().unwrap()),
            Rule::boolean => PropertyValue::Boolean(pair.as_str() == "true"),
            Rule::identifier => PropertyValue::Identifier(pair.as_str().to_string()),
            _ => unreachable!(),
        }
    }
    
    fn parse_string_content(pair: Pair<Rule>) -> String {
        pair.as_str().to_string()
    }
    
    fn unquote(s: &str) -> String {
        s.trim_matches(|c| c == '"' || c == '\'').to_string()
    }
}