#include "slip_dev.h"
#include "pico_stack.h"

static int slip_send(struct pico_device *dev, void *buf, int len)
{
    struct slip_device *slip = (struct slip_device *)dev;
    uint8_t *data = (uint8_t *)buf;
    uint16_t tx_len = 0;

    /* Start with END character */
    slip->tx_buffer[tx_len++] = SLIP_END;

    /* Process each byte */
    for (int i = 0; i < len; i++) {
        switch(data[i]) {
            case SLIP_END:
                slip->tx_buffer[tx_len++] = SLIP_ESC;
                slip->tx_buffer[tx_len++] = SLIP_ESC_END;
                break;
            case SLIP_ESC:
                slip->tx_buffer[tx_len++] = SLIP_ESC;
                slip->tx_buffer[tx_len++] = SLIP_ESC_ESC;
                break;
            default:
                slip->tx_buffer[tx_len++] = data[i];
                break;
        }

        if (tx_len >= (SLIP_TX_BUFFER_SIZE - 2)) {
            /* Not enough space for remaining data and END character */
            return -1;
        }
    }

    /* End with END character */
    slip->tx_buffer[tx_len++] = SLIP_END;

    /* Transmit the packet */
    if (HAL_UART_Transmit_DMA(slip->huart, slip->tx_buffer, tx_len) != HAL_OK) {
        return -1;
    }

    return len;
}

static int slip_poll(struct pico_device *dev, int loop_score)
{
    /* Nothing to do here as we process bytes in interrupt */
    return loop_score;
}

int slip_init(struct slip_device *slip, const char *name, UART_HandleTypeDef *huart, uint8_t *mac_addr)
{
    if (!slip || !huart || !mac_addr)
        return -1;

    /* Initialize PicoTCP device */
    if (pico_device_init((struct pico_device *)slip, name, mac_addr) != 0) {
        return -1;
    }

    /* Setup device callbacks */
    slip->pico_dev.send = slip_send;
    slip->pico_dev.poll = slip_poll;
    slip->pico_dev.destroy = NULL; /* No special destroy needed */

    /* Store UART handle */
    slip->huart = huart;
    slip->rx_index = 0;
    slip->escaped = 0;

    /* Start UART reception */
    HAL_UART_Receive_IT(huart, &(slip->rx_buffer[0]), 1);

    return 0;
}

void slip_deinit(struct slip_device *slip)
{
    if (slip) {
        pico_device_destroy(&slip->pico_dev);
    }
}

void slip_process_byte(struct slip_device *slip, uint8_t byte)
{
    if (!slip) return;

    /* Process byte according to SLIP protocol */
    if (slip->escaped) {
        slip->escaped = 0;
        switch(byte) {
            case SLIP_ESC_END:
                byte = SLIP_END;
                break;
            case SLIP_ESC_ESC:
                byte = SLIP_ESC;
                break;
            default:
                /* Invalid escape sequence, discard */
                return;
        }
    } else if (byte == SLIP_ESC) {
        slip->escaped = 1;
        return;
    } else if (byte == SLIP_END) {
        /* Packet complete */
        if (slip->rx_index > 0) {
            /* Pass to PicoTCP */
            pico_stack_recv(&slip->pico_dev, slip->rx_buffer, slip->rx_index);
            slip->rx_index = 0;
        }
        return;
    }

    /* Store byte if there's space */
    if (slip->rx_index < SLIP_RX_BUFFER_SIZE) {
        slip->rx_buffer[slip->rx_index++] = byte;
    } else {
        /* Buffer overflow, discard packet */
        slip->rx_index = 0;
    }

    /* Restart UART reception for next byte */
    HAL_UART_Receive_IT(slip->huart, &(slip->rx_buffer[slip->rx_index]), 1);
}