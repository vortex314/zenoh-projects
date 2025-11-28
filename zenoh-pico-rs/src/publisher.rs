use crate::{error::*, ffi::*, session::Session, keyexpr::KeyExpr};

pub struct Publisher {
    owned: z_owned_publisher_t,
}

impl Publisher {
    pub fn declare(session: &Session, key: &KeyExpr) -> ZpResult<Self> {
        let mut p: z_owned_publisher_t = unsafe { std::mem::zeroed() };
        let rc = unsafe { z_declare_publisher(&mut p, session.loan(), key.loan(), std::ptr::null()) };
        zret(rc)?;
        Ok(Self { owned: p })
    }

    pub fn put(&self, payload: &[u8]) -> ZpResult<()> {
        let mut view: z_view_slice_t = unsafe { std::mem::zeroed() };
        let rc = unsafe { z_view_slice_from_buf(&mut view, payload.as_ptr(), payload.len()) };
        zret(rc)?;

        let mut bytes: z_owned_bytes_t = unsafe { std::mem::zeroed() };
        let rc = unsafe { z_bytes_from_slice(&mut bytes, z_view_slice_loan(&view)) };
        zret(rc)?;

        let rc = unsafe { z_put(z_publisher_loan(&self.owned), z_bytes_loan(&bytes), std::ptr::null()) };
        zret(rc)?;

        unsafe { z_bytes_drop(z_move_bytes(bytes)) };
        Ok(())
    }
}

impl Drop for Publisher {
    fn drop(&mut self) {
        unsafe { z_undeclare_publisher(z_move_publisher(std::ptr::read(&self.owned))) };
    }
}
