use crate::{error::*, ffi::*, keyexpr::KeyExpr, session::Session};
use std::sync::Arc;

pub struct Subscriber {
    owned: z_owned_subscriber_t,
    _ctx: Arc<dyn Fn(&[u8]) + Send + Sync + 'static>,
}

extern "C" fn sample_handler(sample: *const z_loaned_sample_t, ctx: *mut std::ffi::c_void) {
    let f: &Arc<dyn Fn(&[u8]) + Send + Sync> = unsafe { &*(ctx as *const Arc<dyn Fn(&[u8]) + Send + Sync>) };
    unsafe {
        let payload = z_sample_payload(sample);
        let sl = z_bytes_loan(payload);
        let data = z_slice_data(sl);
        let len = z_slice_len(sl);
        let slice = std::slice::from_raw_parts(data, len);
        f(slice);
    }
}

impl Subscriber {
    pub fn declare(session: &Session, key: &KeyExpr, on_sample: Arc<dyn Fn(&[u8]) + Send + Sync + 'static>) -> ZpResult<Self> {
        let mut sub: z_owned_subscriber_t = unsafe { std::mem::zeroed() };
        let mut cl: z_owned_closure_sample_t = unsafe { std::mem::zeroed() };
        unsafe { z_closure_sample(&mut cl, Some(sample_handler), Arc::into_raw(on_sample.clone()) as *mut _) };
        let rc = unsafe { z_declare_subscriber(&mut sub, session.loan(), key.loan(), z_closure_sample_loan(&cl), std::ptr::null()) };
        zret(rc)?;
        unsafe { z_closure_sample_drop(z_move_closure_sample(cl)) };
        Ok(Self { owned: sub, _ctx: on_sample })
    }
}

impl Drop for Subscriber {
    fn drop(&mut self) {
        unsafe { z_undeclare_subscriber(z_move_subscriber(std::ptr::read(&self.owned))) };
    }
}
