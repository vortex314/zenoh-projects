use serde::Serialize;

#[derive(Debug, Clone, Serialize)]
pub enum FieldType {
    Float,
    I32,
    String,
    Enum(String),      // reference to enum name
}

#[derive(Debug, Clone, Serialize)]
pub struct Field {
    pub name: String,
    pub typ: FieldType,
    pub number: i32,   // from .proto
    pub fnv16: u16,    // computed
}

#[derive(Debug, Clone, Serialize)]
pub struct EnumDef {
    pub name: String,
    pub variants: Vec<String>,
}

#[derive(Debug, Clone, Serialize)]
pub struct StructDef {
    pub name: String,
    pub fields: Vec<Field>,
}

#[derive(Debug, Clone, Serialize)]
pub struct Schema {
    pub enums: Vec<EnumDef>,
    pub structs: Vec<StructDef>,
}
