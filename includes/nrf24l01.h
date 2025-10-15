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

#define NRF24_CE_PIN 20
#define NRF24_CSN_PIN 17

#define SPI_MOSI 19
#define SPI_MISO 16
#define SPI_SCK 18

#define NRF24_SPI spi0

typedef struct NRF24_t{
    int ce_pin;
    int cs_pin;

    int miso_pin;
    int mosi_pin;
    int sck_pin;

    spi_inst_t *spi;

    int channel;
    int data_rate;
    int power_level;

    unsigned char tx_address[5];
    unsigned char rx_address[5];
}NRF24_t;

void cs_select(NRF24_t *device);
void cs_deselect(NRF24_t *device);
void ce_enable(NRF24_t *device);
void ce_disable(NRF24_t *device);

void nrf24_WriteReg(NRF24_t *device, uint8_t reg, uint8_t data);
void nrf24_WriteMultiReg(NRF24_t *device, uint8_t reg, uint8_t *data, int size);
uint8_t nrf24_ReadReg (NRF24_t *device, uint8_t reg);
void nrf24_ReadMultiReg(NRF24_t *device, uint8_t reg, uint8_t *data, int size);
void nrfSendCmd(NRF24_t *device, uint8_t cmd);
void NRF24_init(NRF24_t *device);
void NRF24_txMode(NRF24_t *device, uint8_t *addr, uint16_t channel);
uint8_t NRF24_transmit(NRF24_t *device, uint8_t *data);
void NRF24_rxMode(NRF24_t *device, uint8_t *addr, uint16_t channel);
uint8_t isDataAvaliable(NRF24_t *device, int pipenum);
void NRF24_Receive(NRF24_t *device, uint8_t *data);