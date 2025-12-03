# ---- trying to create a static libzenoh-pico.a 
# ----------------------- I give up ----
## End state
I was able to generate the binding
## Issues encountered
- the order of include directories is important as include files with the same name exist across the compiler toolchain tree and the esp-idf tree. Example : core-isa.h