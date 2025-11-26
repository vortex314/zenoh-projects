from esptool.cmds import detect_chip, attach_flash, reset_chip, read_flash, write_flash, erase_region

PORT = "/dev/cu.usbserial-12140"
DATA1 = b"abcd1234"
DATA2 = b"deadbeef"

with detect_chip(PORT) as esp:
    attach_flash(esp)
    erase_region(esp, 0x0, 4096, force=True)
    try:
        write_flash(esp, [(0, DATA1)])  # 24-bit addr
        write_flash(esp, [(0x1000000, DATA2)])  # 32-bit addr
    except Exception as e:
        print(f"Error: {e}")

    out24 = read_flash(esp, 0x0, 16, flash_size="32MB")
    out32 = read_flash(esp, 0x1000000, 16, flash_size="32MB")
    print(f"0x0000000: {out24}")
    print(f"0x1000000: {out32}")

    reset_chip(esp, "hard-reset")
