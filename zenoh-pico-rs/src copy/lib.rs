#![cfg_attr(feature = "no_std", no_std)]
// src/lib.rs
#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
mod ffi;
mod error;
pub mod session;
pub mod keyexpr;
pub mod publisher;
pub mod subscriber;

pub mod prelude {
    pub use crate::session::Session;
    pub use crate::session::SessionBuilder;
    pub use crate::keyexpr::KeyExpr;
    pub use crate::publisher::Publisher;
    pub use crate::subscriber::Subscriber;
}
