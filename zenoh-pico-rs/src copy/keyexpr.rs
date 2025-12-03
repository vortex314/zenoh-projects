use crate::error::*;
use crate::ffi::*;

pub struct KeyExpr {
    owned: z_owned_keyexpr_t,
}

impl KeyExpr {
    pub fn try_from_str(s: &str) -> ZpResult<Self> {
        let c = std::ffi::CString::new(s).unwrap();
        let mut ke: z_owned_keyexpr_t = unsafe { std::mem::zeroed() };
        let rc = unsafe { z_keyexpr_from_str(&mut ke, c.as_ptr()) };
        zret(rc)?;
        Ok(Self { owned: ke })
    }

    pub(crate) fn loan(&self) -> *const z_loaned_keyexpr_t {
        unsafe { z_keyexpr_loan(&self.owned) }
    }
}
