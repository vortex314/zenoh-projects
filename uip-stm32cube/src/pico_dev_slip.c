#include "pico_dev_slip.h"

/* SLIP protocol constants */
#define SLIP_END 0xC0
#define SLIP_ESC 0xDB
#define SLIP_ESC_END 0xDC
#define SLIP_ESC_ESC 0xDD



/* SLIP device structure */
struct pico_device_slip {
    struct pico_device dev;
    uint32_t serial_base; /* Serial port base address */
    uint8_t rx_buffer[1500]; /* Buffer for incoming SLIP frame */
    uint32_t rx_len; /* Current length of received data */
    uint8_t rx_state; /* SLIP decoding state */
};

/* Forward declarations */
int pico_slip_send(struct pico_device *dev, void* data, int len);
int pico_slip_poll(struct pico_device *dev, int loop_score);

/* Create and initialize SLIP device */
struct pico_device *pico_slip_create(const char *name, uint32_t serial_base, uint32_t baudrate)
{
    struct pico_device_slip *slip_dev;
    
    /* Allocate device structure */
    slip_dev = (struct pico_device_slip *)PICO_ZALLOC(sizeof(struct pico_device_slip));
    if (!slip_dev) {
        return NULL;
    }

    /* Initialize serial hardware */
    if (BSP_serial_init(serial_base, baudrate) != 0) {
        PICO_FREE(slip_dev);
        return NULL;
    }

    /* Initialize pico_device structure */
    slip_dev->dev.send = pico_slip_send;
    slip_dev->dev.poll = pico_slip_poll;
    slip_dev->serial_base = serial_base;
    slip_dev->rx_len = 0;
    slip_dev->rx_state = 0; /* 0: waiting for END, 1: receiving data */

    /* Register device with picoTCP */
    if (pico_device_init((struct pico_device *)slip_dev, name, NULL) != 0) {
        PICO_FREE(slip_dev);
        return NULL;
    }

    return (struct pico_device *)slip_dev;
}
    uint8_t slip_buffer[3000]; /* Worst-case: each byte escaped + END markers */

/* Send function: Encapsulate frame in SLIP and send over serial */
 int pico_slip_send(struct pico_device *dev, void *buf, int len)
{
    struct pico_device_slip *slip_dev = (struct pico_device_slip *)dev;
    uint8_t *data = (uint8_t*)buf;
    uint32_t slip_len = 0;
    int i;

    /* Add SLIP framing */
    slip_buffer[slip_len++] = SLIP_END; /* Start with END */

    for (i = 0; i < len; i++) {
        if (data[i] == SLIP_END) {
            slip_buffer[slip_len++] = SLIP_ESC;
            slip_buffer[slip_len++] = SLIP_ESC_END;
        } else if (data[i] == SLIP_ESC) {
            slip_buffer[slip_len++] = SLIP_ESC;
            slip_buffer[slip_len++] = SLIP_ESC_ESC;
        } else {
            slip_buffer[slip_len++] = data[i];
        }
    }

    slip_buffer[slip_len++] = SLIP_END; /* End with END */

    /* Send over serial */
    if (BSP_serial_write(slip_dev->serial_base, slip_buffer, slip_len) != slip_len) {
        return 0; /* Failed to send */
    }

    /* picoTCP frees the frame immediately after send */
    return len;
}

/* Poll function: Read serial data and decode SLIP frames */
 int pico_slip_poll(struct pico_device *dev, int loop_score)
{
    struct pico_device_slip *slip_dev = (struct pico_device_slip *)dev;
    uint8_t temp_buffer[256];
    int bytes_read;
    int i;

    while (loop_score > 0) {
        /* Read available serial data */
        bytes_read = BSP_serial_read(slip_dev->serial_base, temp_buffer, sizeof(temp_buffer));
        if (bytes_read <= 0) {
            break; /* No data available */
        }

        /* Process received bytes */
        for (i = 0; i < bytes_read; i++) {
            uint8_t byte = temp_buffer[i];

            if (slip_dev->rx_state == 0) {
                /* Waiting for SLIP_END */
                if (byte == SLIP_END) {
                    slip_dev->rx_len = 0;
                    slip_dev->rx_state = 1; /* Start receiving data */
                }
                continue;
            }

            /* Receiving data */
            if (byte == SLIP_END) {
                /* End of frame */
                if (slip_dev->rx_len > 0) {
                    /* Allocate frame for picoTCP */
                    struct pico_frame *f = pico_frame_alloc(slip_dev->rx_len);
                    if (f) {
                        memcpy(f->buffer, slip_dev->rx_buffer, slip_dev->rx_len);
                        f->buffer_len = slip_dev->rx_len;
                        pico_stack_recv_zerocopy(dev, f->buffer, f->buffer_len);
                        loop_score--;
                    }
                }
                slip_dev->rx_len = 0;
                slip_dev->rx_state = 1; /* Ready for next frame */
            } else if (byte == SLIP_ESC) {
                slip_dev->rx_state = 2; /* Expecting escape sequence */
            } else if (slip_dev->rx_state == 2) {
                /* Handle escaped characters */
                if (byte == SLIP_ESC_END) {
                    slip_dev->rx_buffer[slip_dev->rx_len++] = SLIP_END;
                } else if (byte == SLIP_ESC_ESC) {
                    slip_dev->rx_buffer[slip_dev->rx_len++] = SLIP_ESC;
                }
                slip_dev->rx_state = 1; /* Back to normal receiving */
            } else {
                /* Normal byte */
                if (slip_dev->rx_len < sizeof(slip_dev->rx_buffer)) {
                    slip_dev->rx_buffer[slip_dev->rx_len++] = byte;
                }
            }
        }
    }

    return loop_score;
}

/* Optional destroy function */
void pico_slip_destroy(struct pico_device *dev)
{
    struct pico_device_slip *slip_dev = (struct pico_device_slip *)dev;
    pico_device_destroy(dev);
    PICO_FREE(slip_dev);
}