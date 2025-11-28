use thiserror::Error;

#[derive(Debug, Error)]
pub enum ZpError {
    #[error("zenoh-pico error code {0}")]
    Code(i32),
    #[error("null pointer from C API ({0})")]
    Null(&'static str),
}

pub type ZpResult<T> = Result<T, ZpError>;

#[inline]
pub(crate) fn zret(code: i32) -> ZpResult<()> {
    if code < 0 { Err(ZpError::Code(code)) } else { Ok(()) }
}
