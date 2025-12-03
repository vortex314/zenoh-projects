set -x
COMP=$IDF_PATH/components
XTA=~/.espressif/tools/xtensa-esp-elf/esp-14.2.0_20241119/xtensa-esp-elf
bindgen zenoh-pico/include/zenoh-pico.h -o src/bindings.rs \
    --allowlist-function '^(zenoh_.*|z_.*)$' \
    -- \
    -I./config \
    -I./include \
    -I./src \
    -I$XTA/lib/gcc/xtensa-esp-elf/14.2.0/include \
    -I$COMP/esp_driver_uart/include \
    -I$COMP/esp_common/include \
    -I$COMP/esp_hw_support/include \
    -I$COMP/esp_rom/include \
    -I$COMP/esp_system/include \
    -I$COMP/esp_flash/include \
    -I$COMP/esp_chip_info/include \
    -I$COMP/esp_cpu/include \
    -I$COMP/soc/esp32/include \
    -I$COMP/soc/esp32/register \
    -I$COMP/hal/include \
    -I$COMP/freertos/FreeRTOS-Kernel/include \
    -I$COMP/freertos/config/include/freertos \
    -I$COMP/freertos/config/xtensa/include \
    -I$COMP/freertos/FreeRTOS-Kernel/portable/xtensa/include/freertos \
    -I$COMP/xtensa/include \
    -I$COMP/xtensa/esp32/include \
    -I$COMP/xtensa/esp32/include/xtensa/config \
    -I$XTA/xtensa-esp-elf/include \
    -Izenoh-pico/include \
    -I/home/lieven/esp/esp-idf/components/newlib/platform_include \
    -I/home/lieven/esp/esp-idf/components/heap/include \
    -I/home/lieven/esp/esp-idf/components/log/include \
    -I/home/lieven/esp/esp-idf/components/soc/include \
    -DZENOH_ESPIDF \
    -D_GLIBCXX_HAVE_STDINT_H \
    -D__XTENSA__


exit
    

        -I$XTA/xtensa-esp-elf/include \
    -I$XTA/lib/gcc/xtensa-esp-elf/14.2.0/include \
        -I$XTA/lib/gcc/xtensa-esp-elf/14.2.0/include \
    -I$XTA/xtensa-esp-elf/include/c++/14.2.0/tr1/ \
    -I$XTA/xtensa-esp-elf/include/c++/14.2.0/ \
    -I$XTA/xtensa-esp-elf/include/c++/14.2.0/xtensa-esp-elf/ \