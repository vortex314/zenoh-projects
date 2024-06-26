#![allow(clippy::too_many_arguments)]

pub mod error;
// This file was code-generated using an experimental CDDL to rust tool:
// https://github.com/dcSpark/cddl-codegen

pub mod serialization;

use crate::error::*;
use std::collections::BTreeMap;
use std::convert::TryFrom;

#[derive(Copy, Eq, PartialEq, Ord, PartialOrd, Clone, Debug)]
#[wasm_bindgen::prelude::wasm_bindgen]
pub enum Level {
    Error,
    Warning,
    Info,
    Debug,
    Trace,
}
