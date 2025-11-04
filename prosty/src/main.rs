use convert_case::Casing;
use protobuf_parser::{FieldType, FileDescriptor};
use std::path::Path;
use std::fs;
mod logger;
use log::{error, info};
use logger::init;

// parse CLI line for proto file path and output directory
#[derive(Copy, Clone, PartialEq, Eq, PartialOrd, Ord, ValueEnum, Debug)]
enum Lang {
    Cpp,
    Rust,
}
impl Lang {
    fn as_str(&self) -> &'static str {
        match self {
            Lang::Cpp => "cpp",
            Lang::Rust => "rust",
        }
    }
}
#[derive(Copy, Clone, PartialEq, Eq, PartialOrd, Ord, ValueEnum, Debug)]
enum Format {
    Json,
    Cbor,
    Mix,
}

impl Format {
    fn as_str(&self) -> &'static str {
        match self {
            Format::Json => "json",
            Format::Cbor => "cbor",
            Format::Mix => "mix",
        }
    }
}


use clap::{ Parser,ValueEnum };
#[derive(Parser, Debug)]
#[command(author, version, about, long_about = None)]
struct Args {
    /// Path to the .proto file
    /// default: proto/message.proto
    /// example: --proto proto/message.proto
    #[arg(short, long, default_value = "proto/message.proto")]
    input: String,
    /// Output directory for the generated files
    /// default: examples
    #[arg(short, long, default_value = "../zenoh-esp32-eventbus/src")]
    output: String,

    #[arg(short, long, value_enum,default_value_t = Lang::Rust)]
    lang: Lang,
    #[arg(short, long, value_enum, default_value_t = Format::Json)]
    format: Format,
}

fn main() -> anyhow::Result<()> {
    let args = Args::parse();
    /*  pkg = Path::new(&args.proto)
        .parent()
        .expect("Failed to get parent directory");*/
    init();
    let proto_file = Path::new(&args.input);
    let proto_content = fs::read_to_string(proto_file).expect("Failed to read proto file");

    let res = FileDescriptor::parse(&proto_content.as_bytes());
    if res.is_err() {
        panic!("Failed to parse proto file: {:?}", res.err());
    }
    let fd = res.unwrap();
    info!("Proto file parsed successfully.");
    let tera_file = format!("{}_{}.tera", args.lang.as_str(), args.format.as_str());
    info!("Using template file: {}", tera_file);

    if args.lang == Lang::Rust {
        info!("Generating Rust code in {} format to {}", args.format.as_str(), args.output  );
        let messages = convert_rust_types(&fd);
        let enums = convert_enum_rust_types(&fd);
        let rust_name = format!("{}/{}.rs", args.output, fd.package);
        let rendered = render(&enums, &messages, tera_file.as_str());
        fs::write(&rust_name, rendered).expect("Failed to write output file");
        info!("Generated Rust code written to {}", rust_name);
    } else if args.lang == Lang::Cpp {
        info!("Generating C++ code in {} format to {}", args.format.as_str(), args.output);
        let messages = convert_cpp_types(&fd);
        let enums = convert_enum_cpp_types(&fd);
        let cpp_name = format!("{}/{}.cpp", args.output, fd.package);
        let rendered = render(&enums, &messages, tera_file.as_str());
        fs::write(&cpp_name, rendered).expect("Failed to write output file");
        info!("Generated C++ code written to {}", cpp_name);
    } else {
        error!("Unsupported language: {:?}", args.lang);
    }
    Ok(())
}

use serde::Serialize;

#[derive(Serialize)]
struct Field {
    name: String,
    index: i32,
    target_type: String,
    repeated: bool,
    optional: bool,
}

#[derive(Serialize)]
struct Message {
    name: String,
    fields: Vec<Field>,
    msg_id: u32,
}
#[derive(Serialize)]
struct EnumType {
    name: String,
    values: Vec<(String, i32)>,
}

fn field_type_to_rust_type(field_type: &FieldType) -> String {
    match field_type {
        FieldType::Float => "f32".to_string(),
        FieldType::Int32 => "i32".to_string(),
        FieldType::Int64 => "i64".to_string(),
        FieldType::Uint32 => "u32".to_string(),
        FieldType::Uint64 => "u64".to_string(),
        FieldType::String => "String".to_string(),
        FieldType::MessageOrEnum(msg_name) => msg_name.clone(),
        FieldType::Map(_other_name) => format!(
            "std::collections::HashMap<{},{}>",
            field_type_to_rust_type(&_other_name.0),
            field_type_to_rust_type(&_other_name.1)
        )
        .to_string(),
        FieldType::Bytes => "Vec<u8>".to_string(),
        FieldType::Bool => "bool".to_string(),
        FieldType::Double => "f64".to_string(),
        FieldType::Sint32 => "i32".to_string(),
        FieldType::Sint64 => "i64".to_string(),
        FieldType::Fixed32 => "u32".to_string(),
        FieldType::Fixed64 => "u64".to_string(),
        FieldType::Sfixed32 => "i32".to_string(),
        FieldType::Sfixed64 => "i64".to_string(),
        FieldType::Group(_) => "/* group */".to_string(),
    }
}

fn field_type_to_cpp_type(field_type: &FieldType) -> String {
    match field_type {
        FieldType::Float => "float".to_string(),
        FieldType::Int32 => "int32_t".to_string(),
        FieldType::Int64 => "int64_t".to_string(),
        FieldType::Uint32 => "uint32_t".to_string(),
        FieldType::Uint64 => "uint64_t".to_string(),
        FieldType::String => "std::string".to_string(),
        FieldType::MessageOrEnum(msg_name) => msg_name.clone(),
        FieldType::Map(_other_name) => format!(
            "std::unordered_map<{},{}>",
            field_type_to_cpp_type(&_other_name.0),
            field_type_to_cpp_type(&_other_name.1)
        )
        .to_string(),
        FieldType::Bytes => "Bytes".to_string(),
        FieldType::Bool => "bool".to_string(),
        FieldType::Double => "double".to_string(),
        FieldType::Sint32 => "int32_t".to_string(),
        FieldType::Sint64 => "int64_t".to_string(),
        FieldType::Fixed32 => "uint32_t".to_string(),
        FieldType::Fixed64 => "uint64_t".to_string(),
        FieldType::Sfixed32 => "int32_t".to_string(),
        FieldType::Sfixed64 => "int64_t".to_string(),
        FieldType::Group(_) => "/* group */".to_string(),
    }
}

fn convert_rust_types(fd: &FileDescriptor) -> Vec<Message> {
    fd.messages
        .iter()
        .map(|msg| {
            let fields = msg
                .fields
                .iter()
                .map(|f| Field {
                    name: f.name.clone(),
                    index : f.number,
                    target_type: field_type_to_rust_type(&f.typ),
                    repeated: match f.rule {
                        protobuf_parser::Rule::Repeated => true,
                        _ => false,
                    },
                    optional: match f.rule {
                        protobuf_parser::Rule::Optional => true,
                        _ => false,
                    },
                })
                .collect();

            Message {
                name: msg.name.clone(),
                fields,
                msg_id: fnv1a_16(&msg.name.as_bytes()) as u32,
            }
        })
        .collect()
}

fn convert_cpp_types(fd: &FileDescriptor) -> Vec<Message> {
    fd.messages
        .iter()
        .map(|msg| {
            let fields = msg
                .fields
                .iter()
                .map(|f| Field {
                    name: f.name.clone(),
                    index : f.number,
                    target_type: field_type_to_cpp_type(&f.typ),
                    repeated: match f.rule {
                        protobuf_parser::Rule::Repeated => true,
                        _ => false,
                    },
                    optional: match f.rule {
                        protobuf_parser::Rule::Optional => true,
                        _ => false,
                    },
                })
                .collect();

            Message {
                name: msg.name.clone(),
                fields,
                msg_id: fnv1a_16(&msg.name.as_bytes()) as u32,
            }
        })
        .collect()
}

fn convert_enum_rust_types(fd: &FileDescriptor) -> Vec<EnumType> {
    let enums: Vec<EnumType> = fd
        .enums
        .iter()
        .map(|e| {
            let values: Vec<(String, i32)> = e
                .values
                .iter()
                .map(|v| (v.name.clone().to_case(convert_case::Case::Pascal), v.number))
                .collect();
            EnumType {
                name: e.name.clone(),
                values,
            }
        })
        .collect();
    enums
}

fn convert_enum_cpp_types(fd: &FileDescriptor) -> Vec<EnumType> {
    let enums: Vec<EnumType> = fd
        .enums
        .iter()
        .map(|e| {
            let values: Vec<(String, i32)> = e
                .values
                .iter()
                .map(|v| {
                    (
                        v.name.clone().to_case(convert_case::Case::Constant),
                        v.number,
                    )
                })
                .collect();
            EnumType {
                name: e.name.clone(),
                values,
            }
        })
        .collect();
    enums
}

use tera::{Context, Tera};
/* 
fn fnv1a_32(data: &[u8]) -> u32 {
    const FNV_OFFSET_BASIS: u32 = 0x811c9dc5;
    const FNV_PRIME: u32 = 0x01000193;
    let mut hash = FNV_OFFSET_BASIS;
    for byte in data {
        hash ^= *byte as u32;
        hash = hash.wrapping_mul(FNV_PRIME);
    }
    hash
}*/

fn fnv1a_16(data: &[u8]) -> u16 {
    const FNV_OFFSET_BASIS: u16 = 0x811c;
    const FNV_PRIME: u16 = 0x0101;
    let mut hash = FNV_OFFSET_BASIS;
    for byte in data {
        hash ^= *byte as u16;
        hash = hash.wrapping_mul(FNV_PRIME);
    }
    hash
}

fn render(enums: &Vec<EnumType>, messages: &Vec<Message>, tera_file: &str) -> String {
    let tera = Tera::new("src/*.tera").unwrap();
    let mut context = Context::new();
    context.insert("messages", messages);
    context.insert("enums", enums);
    tera.render(tera_file, &context).unwrap()
}
