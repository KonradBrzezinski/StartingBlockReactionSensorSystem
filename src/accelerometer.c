/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "nrf24l01.h"

#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0') 


int main() {

    stdio_init_all();

    spi_inst_t *spi = spi0;
    nrf24_init(spi, 1000 * 1000);

    nrf24_init_tx(spi);

    sleep_ms(100);

    printf("TEST\n");
    uint8_t config_reg = nrf24_read_reg(spi, 0x00);
    printf("Odczytany rejestr CONFIG: 0x%X\n", config_reg);

    while(1){
        // receive_msg(spi);
        send_msg(spi, "TEST");
        sleep_ms(1000);
    }
}