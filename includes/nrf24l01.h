#pragma once

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "nrf24l01_map.h"
#include "string.h"

static const uint8_t SPI_SCK_PIN = 18;
static const uint8_t SPI_TX_PIN = 19; //MOSI pin
static const uint8_t SPI_RX_PIN = 16; //MISO pin
static const uint8_t SPI_CSN_PIN = 17;
static const uint8_t NRF24_CE_PIN = 20;

void csn_select(void);
void csn_deselect(void);

void nrf24_init(spi_inst_t *spi, uint32_t baudrate);
void nrf24_write_reg(spi_inst_t *spi, uint8_t reg, uint8_t value);
uint8_t nrf24_read_reg(spi_inst_t *spi, uint8_t reg);
void nrf24_init_tx(spi_inst_t *spi);
void nrf24_init_rx(spi_inst_t *spi);
void nrf24_write_payload(spi_inst_t *spi, uint8_t *data, int len);
void nrf24_read_payload(spi_inst_t *spi, uint8_t *data, int len);
void send_msg(spi_inst_t *spi, char *msg);
void receive_msg(spi_inst_t *spi);