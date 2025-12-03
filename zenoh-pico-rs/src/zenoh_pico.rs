use super::ffi;
use std::ffi::{CString, CStr};
use std::ptr;
use std::os::raw::c_char;
use std::sync::Arc;

pub type Result<T> = std::result::Result<T, ZenohError>;

#[derive(Debug)]
pub enum ZenohError {
    InitializationFailed,
    OpenFailed,
    PublishFailed,
    SubscribeFailed,
    InvalidString,
    TaskStartFailed,
    TaskStopFailed,
}

/// Safe wrapper for z_owned_config_t
pub struct Config {
    inner: ffi::z_owned_config_t,
}

impl Config {
    pub fn default() -> Self {
        unsafe {
            let mut config = std::mem::zeroed();
            ffi::z_config_default(&mut config);
            Config { inner: config }
        }
    }
    
    pub fn client(locators: &[&str]) -> Result<Self> {
        let mut config = Self::default();
        
        for locator in locators {
            let c_locator = CString::new(*locator)
                .map_err(|_| ZenohError::InvalidString)?;
            
            unsafe {
                let mode = CString::new("client").unwrap();
                ffi::z_config_insert(
                    ffi::z_config_loan_mut(&mut config.inner),
                    ffi::z_config_mode_key(),
                    mode.as_ptr()
                );
                
                ffi::z_config_insert(
                    ffi::z_config_loan_mut(&mut config.inner),
                    ffi::z_config_connect_key(),
                    c_locator.as_ptr()
                );
            }
        }
        
        Ok(config)
    }
    
    pub fn peer(listen: &[&str]) -> Result<Self> {
        let mut config = Self::default();
        
        unsafe {
            let mode = CString::new("peer").unwrap();
            ffi::z_config_insert(
                ffi::z_config_loan_mut(&mut config.inner),
                ffi::z_config_mode_key(),
                mode.as_ptr()
            );
        }
        
        for locator in listen {
            let c_locator = CString::new(*locator)
                .map_err(|_| ZenohError::InvalidString)?;
            
            unsafe {
                ffi::z_config_insert(
                    ffi::z_config_loan_mut(&mut config.inner),
                    ffi::z_config_listen_key(),
                    c_locator.as_ptr()
                );
            }
        }
        
        Ok(config)
    }
}

impl Drop for Config {
    fn drop(&mut self) {
        unsafe {
            ffi::z_config_drop(&mut self.inner);
        }
    }
}

/// Safe wrapper for z_owned_session_t with task management
pub struct Session {
    inner: ffi::z_owned_session_t,
    read_task_running: bool,
    lease_task_running: bool,
}

impl Session {
    /// Open a Zenoh session
    pub fn open(config: Config) -> Result<Self> {
        unsafe {
            let mut session = std::mem::zeroed();
            let mut config_inner = config.inner;
            
            let result = ffi::z_open(
                &mut session,
                ffi::z_config_move(&mut config_inner),
                ptr::null()
            );
            
            if result != 0 || !ffi::z_session_check(&session) {
                return Err(ZenohError::OpenFailed);
            }
            
            std::mem::forget(config); // Moved into session
            Ok(Session {
                inner: session,
                read_task_running: false,
                lease_task_running: false,
            })
        }
    }
    
    /// Start the read task
    /// This task handles incoming messages from the network
    pub fn start_read_task(&mut self) -> Result<()> {
        if self.read_task_running {
            return Ok(()); // Already running
        }
        
        unsafe {
            let result = ffi::zp_start_read_task(
                ffi::z_session_loan_mut(&mut self.inner),
                ptr::null()
            );
            
            if result != 0 {
                return Err(ZenohError::TaskStartFailed);
            }
            
            self.read_task_running = true;
            Ok(())
        }
    }
    
    /// Start the lease task
    /// This task sends keep-alive messages to maintain the session
    pub fn start_lease_task(&mut self) -> Result<()> {
        if self.lease_task_running {
            return Ok(()); // Already running
        }
        
        unsafe {
            let result = ffi::zp_start_lease_task(
                ffi::z_session_loan_mut(&mut self.inner),
                ptr::null()
            );
            
            if result != 0 {
                return Err(ZenohError::TaskStartFailed);
            }
            
            self.lease_task_running = true;
            Ok(())
        }
    }
    
    /// Stop the read task
    pub fn stop_read_task(&mut self) -> Result<()> {
        if !self.read_task_running {
            return Ok(());
        }
        
        unsafe {
            let result = ffi::zp_stop_read_task(
                ffi::z_session_loan_mut(&mut self.inner)
            );
            
            if result != 0 {
                return Err(ZenohError::TaskStopFailed);
            }
            
            self.read_task_running = false;
            Ok(())
        }
    }
    
    /// Stop the lease task
    pub fn stop_lease_task(&mut self) -> Result<()> {
        if !self.lease_task_running {
            return Ok(());
        }
        
        unsafe {
            let result = ffi::zp_stop_lease_task(
                ffi::z_session_loan_mut(&mut self.inner)
            );
            
            if result != 0 {
                return Err(ZenohError::TaskStopFailed);
            }
            
            self.lease_task_running = false;
            Ok(())
        }
    }
    
    /// For single-threaded mode: manually trigger read operations
    pub fn read(&self) -> Result<()> {
        unsafe {
            let result = ffi::zp_read(
                ffi::z_session_loan(&self.inner),
                ptr::null()
            );
            
            if result != 0 {
                return Err(ZenohError::InitializationFailed);
            }
            
            Ok(())
        }
    }
    
    /// For single-threaded mode: manually trigger lease operations
    pub fn send_keep_alive(&self) -> Result<()> {
        unsafe {
            let result = ffi::zp_send_keep_alive(
                ffi::z_session_loan(&self.inner),
                ptr::null()
            );
            
            if result != 0 {
                return Err(ZenohError::InitializationFailed);
            }
            
            Ok(())
        }
    }
    
    /// For single-threaded mode: manually trigger join operations
    pub fn send_join(&self) -> Result<()> {
        unsafe {
            let result = ffi::zp_send_join(
                ffi::z_session_loan(&self.inner),
                ptr::null()
            );
            
            if result != 0 {
                return Err(ZenohError::InitializationFailed);
            }
            
            Ok(())
        }
    }
    
    pub fn as_loan(&self) -> *const ffi::z_loaned_session_t {
        unsafe { ffi::z_session_loan(&self.inner) }
    }
    
    pub fn as_loan_mut(&mut self) -> *mut ffi::z_loaned_session_t {
        unsafe { ffi::z_session_loan_mut(&mut self.inner) }
    }
}

impl Drop for Session {
    fn drop(&mut self) {
        // Stop tasks before closing session
        let _ = self.stop_read_task();
        let _ = self.stop_lease_task();
        
        // Add a small delay to allow tasks to clean up
        // This addresses the racing condition mentioned in the GitHub issues
        std::thread::sleep(std::time::Duration::from_millis(100));
        
        unsafe {
            ffi::z_session_drop(&mut self.inner);
        }
    }
}

/// Safe wrapper for publishing
pub struct Publisher {
    inner: ffi::z_owned_publisher_t,
}

impl Publisher {
    pub fn new(session: &Session, key_expr: &str) -> Result<Self> {
        let c_key = CString::new(key_expr)
            .map_err(|_| ZenohError::InvalidString)?;
        
        unsafe {
            let mut publisher = std::mem::zeroed();
            let keyexpr = ffi::z_keyexpr(c_key.as_ptr());
            
            let result = ffi::z_publisher_declare(
                &mut publisher,
                session.as_loan(),
                keyexpr,
                ptr::null()
            );
            
            if result != 0 || !ffi::z_publisher_check(&publisher) {
                return Err(ZenohError::PublishFailed);
            }
            
            Ok(Publisher { inner: publisher })
        }
    }
    
    pub fn put(&self, payload: &[u8]) -> Result<()> {
        unsafe {
            let mut options = ffi::z_publisher_put_options_default();
            
            let mut bytes = std::mem::zeroed();
            ffi::z_bytes_from_buf(
                &mut bytes,
                payload.as_ptr(),
                payload.len(),
                None,
                ptr::null_mut()
            );
            
            let result = ffi::z_publisher_put(
                ffi::z_publisher_loan(&self.inner),
                ffi::z_bytes_move(&mut bytes),
                &options
            );
            
            if result != 0 {
                return Err(ZenohError::PublishFailed);
            }
            
            Ok(())
        }
    }
}

impl Drop for Publisher {
    fn drop(&mut self) {
        unsafe {
            ffi::z_publisher_drop(&mut self.inner);
        }
    }
}

/// Subscriber callback wrapper
pub struct Subscriber {
    inner: ffi::z_owned_subscriber_t,
    _callback_data: Box<SubscriberCallbackData>,
}

struct SubscriberCallbackData {
    callback: Box<dyn FnMut(&[u8]) + Send + 'static>,
}

extern "C" fn subscriber_callback(
    sample: *const ffi::z_loaned_sample_t,
    context: *mut std::os::raw::c_void
) {
    unsafe {
        let callback_data = &mut *(context as *mut SubscriberCallbackData);
        
        let payload = ffi::z_sample_payload(sample);
        let mut iter = std::mem::zeroed();
        ffi::z_bytes_get_iterator(payload, &mut iter);
        
        let mut data = Vec::new();
        let mut slice = std::mem::zeroed();
        
        while ffi::z_bytes_iterator_next(&mut iter, &mut slice) {
            let slice_data = ffi::z_slice_data(ffi::z_view_slice_loan(&slice));
            let slice_len = ffi::z_slice_len(ffi::z_view_slice_loan(&slice));
            
            let slice_bytes = std::slice::from_raw_parts(slice_data, slice_len);
            data.extend_from_slice(slice_bytes);
        }
        
        (callback_data.callback)(&data);
    }
}

impl Subscriber {
    pub fn new<F>(session: &Session, key_expr: &str, callback: F) -> Result<Self>
    where
        F: FnMut(&[u8]) + Send + 'static
    {
        let c_key = CString::new(key_expr)
            .map_err(|_| ZenohError::InvalidString)?;
        
        let callback_data = Box::new(SubscriberCallbackData {
            callback: Box::new(callback),
        });
        
        let context = Box::into_raw(callback_data) as *mut std::os::raw::c_void;
        
        unsafe {
            let mut subscriber = std::mem::zeroed();
            let keyexpr = ffi::z_keyexpr(c_key.as_ptr());
            
            let mut closure = std::mem::zeroed();
            ffi::z_closure_sample(
                &mut closure,
                Some(subscriber_callback),
                None,
                context
            );
            
            let result = ffi::z_subscriber_declare(
                &mut subscriber,
                session.as_loan(),
                keyexpr,
                ffi::z_closure_sample_move(&mut closure),
                ptr::null()
            );
            
            if result != 0 || !ffi::z_subscriber_check(&subscriber) {
                let _ = Box::from_raw(context as *mut SubscriberCallbackData);
                return Err(ZenohError::SubscribeFailed);
            }
            
            let callback_data = Box::from_raw(context as *mut SubscriberCallbackData);
            
            Ok(Subscriber {
                inner: subscriber,
                _callback_data: callback_data,
            })
        }
    }
}

impl Drop for Subscriber {
    fn drop(&mut self) {
        unsafe {
            ffi::z_subscriber_drop(&mut self.inner);
        }
    }
}