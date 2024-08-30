# History 
### combining esp-idf-svc std and embassy ==> BAD IDEA !!
- fails because of critical-section crate conflicts in features 
```
64 | compile_error!("You must set at most one of these Cargo features: restore-state-none, restore-state-bool, restore-state-u8, restore-state-u16, restore-state-u32, restore-state-u64...
```
In the embassy esp32 examples the peripherals (esp-hal) provide a 'SYSTEM' field , which is not provided in esp-idf-hal
### using crate futures 