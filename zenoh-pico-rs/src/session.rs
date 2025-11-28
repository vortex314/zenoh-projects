use crate::error::*;
use crate::ffi::*;

pub struct Session {
    owned: z_owned_session_t,
}

impl Session {
    pub fn poll(&self) -> ZpResult<()> {
        let rc = unsafe { zp_read(self.loan()) };
        zret(rc)
    }

    pub(crate) fn loan(&self) -> *const z_loaned_session_t {
        unsafe { z_session_loan(&self.owned) }
    }
}

impl Drop for Session {
    fn drop(&mut self) {
        unsafe { z_close(z_move_session(std::ptr::read(&self.owned)), std::ptr::null()) };
    }
}

pub struct SessionBuilder {
    cfg: z_owned_config_t,
}

impl SessionBuilder {
    pub fn new() -> ZpResult<Self> {
        let mut cfg: z_owned_config_t = unsafe { std::mem::zeroed() };
        zret(unsafe { z_config_default(&mut cfg) })?;
        Ok(Self { cfg })
    }

    pub fn mode_peer(mut self) -> Self {
        unsafe { z_config_set_mode_peer(z_config_loan(&self.cfg)) };
        self
    }

    pub fn add_listen(mut self, endpoint: &str) -> ZpResult<Self> {
        let c = std::ffi::CString::new(endpoint).unwrap();
        zret(unsafe { z_config_add_listen(z_config_loan(&self.cfg), c.as_ptr()) })?;
        Ok(self)
    }

    pub fn add_connect(mut self, endpoint: &str) -> ZpResult<Self> {
        let c = std::ffi::CString::new(endpoint).unwrap();
        zret(unsafe { z_config_add_connect(z_config_loan(&self.cfg), c.as_ptr()) })?;
        Ok(self)
    }

    pub fn build(self) -> ZpResult<Session> {
        let mut sess: z_owned_session_t = unsafe { std::mem::zeroed() };
        zret(unsafe { z_open(&mut sess, z_move_config(self.cfg), std::ptr::null()) })?;
        Ok(Session { owned: sess })
    }
}
