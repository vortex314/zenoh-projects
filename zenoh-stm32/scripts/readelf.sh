FIRMWARE_ELF="..pio/build/stm32_stm32cube/firmware.elf"

arm-none-eabi-readelf -S $FIRMWARE_ELF

size $FIRMWARE_ELF

objdump -x $FIRMWARE_ELF |grep -i -A 1 bss

objdump -C -d -S -t -j .bss   $FIRMWARE_ELF

nm -n -S --size-sort $FIRMWARE_ELF | grep -i bss

nm -C -S -t d -s .bss $FIRMWARE_ELF