#![allow(clippy::too_many_arguments)]

pub mod error;
// This file was code-generated using an experimental CDDL to rust tool:
// https://github.com/dcSpark/cddl-codegen

pub mod serialization;

use crate::error::*;
use std::collections::BTreeMap;
use std::convert::TryFrom;

#[derive(Clone, Debug)]
pub struct Bar {
    pub foo: Foo,
    pub derp: Option<u64>,
    pub key_1: Option<u64>,
    pub key_5: Option<String>,
}

impl Bar {
    pub fn new(foo: Foo, key_1: Option<u64>) -> Self {
        Self {
            foo,
            derp: None,
            key_1,
            key_5: None,
        }
    }
}

#[derive(Clone, Debug)]
pub struct Foo {
    pub index_0: u64,
    pub index_1: String,
    pub index_2: Vec<u8>,
}

impl Foo {
    pub fn new(index_0: u64, index_1: String, index_2: Vec<u8>) -> Self {
        Self {
            index_0,
            index_1,
            index_2,
        }
    }
}

#[derive(Clone, Debug)]
pub struct Foo2 {
    pub index_0: u64,
    pub opt_text: OptText,
}

impl Foo2 {
    pub fn new(index_0: u64, opt_text: OptText) -> Self {
        Self { index_0, opt_text }
    }
}

#[derive(Clone, Debug)]
pub enum GroupChoice {
    Foo(Foo),
    GroupChoice1(u64),
    Plain(Plain),
}

impl GroupChoice {
    pub fn new_foo(index_0: u64, index_1: String, index_2: Vec<u8>) -> Self {
        Self::Foo(Foo::new(index_0, index_1, index_2))
    }

    pub fn new_group_choice1(x: u64) -> Self {
        Self::GroupChoice1(x)
    }

    pub fn new_plain(d: u64, e: TaggedText) -> Self {
        Self::Plain(Plain::new(d, e))
    }
}

pub type Hash = Vec<u8>;

pub type OptText = Option<TaggedText>;

#[derive(Clone, Debug)]
pub struct Outer {
    pub a: u64,
    pub b: Plain,
}

impl Outer {
    pub fn new(a: u64, b: Plain) -> Self {
        Self { a, b }
    }
}

#[derive(Clone, Debug)]
pub struct Plain {
    pub d: u64,
    pub e: TaggedText,
}

impl Plain {
    pub fn new(d: u64, e: TaggedText) -> Self {
        Self { d, e }
    }
}

pub type Table = BTreeMap<u64, String>;

#[derive(Clone, Debug)]
pub struct TableArrMembers {
    pub tab: BTreeMap<String, String>,
    pub arr: Vec<u64>,
    pub arr2: Vec<Foo>,
}

impl TableArrMembers {
    pub fn new(tab: BTreeMap<String, String>, arr: Vec<u64>, arr2: Vec<Foo>) -> Self {
        Self { tab, arr, arr2 }
    }
}

pub type TaggedText = String;

#[derive(Clone, Debug)]
pub enum TypeChoice {
    I0,
    Helloworld,
    U64(u64),
    Text(String),
    Bytes(Vec<u8>),
    ArrU64(Vec<u64>),
}

impl TypeChoice {
    pub fn new_i0() -> Self {
        Self::I0
    }

    pub fn new_helloworld() -> Self {
        Self::Helloworld
    }

    pub fn new_uint(uint: u64) -> Self {
        Self::U64(uint)
    }

    pub fn new_text(text: String) -> Self {
        Self::Text(text)
    }

    pub fn new_bytes(bytes: Vec<u8>) -> Self {
        Self::Bytes(bytes)
    }

    pub fn new_arr_u64(arr_u64: Vec<u64>) -> Self {
        Self::ArrU64(arr_u64)
    }
}
