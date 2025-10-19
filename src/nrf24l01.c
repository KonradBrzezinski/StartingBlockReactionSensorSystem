#include "includes/nrf24l01.h"

void cs_select(NRF24_t *device){
    gpio_put(device->cs_pin, 0);
    sleep_us(10);
}

void cs_deselect(NRF24_t *device){
    sleep_us(10);
    gpio_put(device->cs_pin, 1);
}

void ce_enable(NRF24_t *device){
    gpio_put(device->ce_pin, 1);
    sleep_us(10);
}

void ce_disable(NRF24_t *device){
    sleep_us(10);
    gpio_put(device->ce_pin, 0);
}

void nrf24_WriteReg(NRF24_t *device, uint8_t reg, uint8_t data){
    uint8_t buf[2];
    buf[0] = reg|1<<5;
    buf[1] = data;

    cs_select(device);

    spi_write_blocking(device, buf, 2);

    cs_deselect(device);

}

void nrf24_WriteMultiReg(NRF24_t *device, uint8_t reg, uint8_t *data, int size){
    uint8_t buf[2];
    buf[0] = reg|1<<5;

    cs_select(device);

    spi_write_blocking(device, buf, 2);
    spi_write_blocking(device, data, size);

    cs_deselect(device);
}

uint8_t nrf24_ReadReg (NRF24_t *device, uint8_t reg){
    uint8_t data = 0;
    cs_select(device);

    spi_write_blocking(device->spi, &reg, 1);
    spi_read_blocking(device->spi, 0x00, &data, 1);

    cs_deselect(device);

    return data;
}

void nrf24_ReadMultiReg(NRF24_t *device, uint8_t reg, uint8_t *data, int size){
    cs_select(device);
    spi_write_blocking(device->spi, &reg, 1);
    spi_read_blocking(device->spi, 0, data, size);
    cs_deselect(device);
}

void nrfSendCmd(NRF24_t *device, uint8_t cmd){
    cs_select(device);

    spi_write_blocking(device->spi, &cmd, 1);

    cs_deselect(device);
}

void NRF24_init(NRF24_t *device){

    spi_init(device->spi, 1000 * 1000);

    gpio_init(device->ce_pin);
    gpio_init(device->cs_pin);
    gpio_init(device->irq_pin);

    gpio_set_dir(device->ce_pin, 1);
    gpio_set_dir(device->cs_pin, 1);
    gpio_set_dir(device->irq_pin, 0);

    gpio_pull_up(device->irq_pin);

    gpio_set_function(device->miso_pin, GPIO_FUNC_SPI);
    gpio_set_function(device->mosi_pin, GPIO_FUNC_SPI);
    gpio_set_function(device->sck_pin,  GPIO_FUNC_SPI);

    ce_disable(device);

    nrf24_WriteReg(device, CONFIG,     0   );
    nrf24_WriteReg(device, EN_AA,      0   );
    nrf24_WriteReg(device, EN_RXADDR,  0   );
    nrf24_WriteReg(device, SETUP_AW,   0x03);
    nrf24_WriteReg(device, SETUP_RETR, 0   );
    nrf24_WriteReg(device, RF_CH,      0   );
    nrf24_WriteReg(device, RF_SETUP,   0x0E);

    ce_enable(device);
}

void NRF24_txMode(NRF24_t *device, uint8_t *addr, uint16_t channel){
    ce_disable(device);

    nrf24_WriteReg(device, RF_CH, channel);
    nrf24_WriteMultiReg(device, TX_ADDR, addr, 5);
    uint8_t config = nrf24_ReadReg(device, CONFIG);
    config = config | (1 << 1);
    nrf24_WriteReg(device, CONFIG, config);


    ce_enable(device);
}

uint8_t NRF24_transmit(NRF24_t *device, uint8_t *data){
    uint8_t cmdToSend = 0;
   
    cs_select(device);

    cmdToSend = W_TX_PAYLOAD;
    // uint8_t cmdsent = spi_write_blocking(NRF24_SPI, &cmdToSend, 1);
    // sleep_ms(5);
    spi_write_blocking(device->spi, &cmdToSend, 1);
    // sleep_ms(5);
    // uint8_t datasent = spi_write_blocking(NRF24_SPI, data, 32);
    spi_write_blocking(device->spi, data, 1); //tu było 32
    // sleep_ms(5);

    cs_deselect(device);

    sleep_us(300);

    // printf("BYTES WRITTEN FOR CMD: %d\n", cmdsent);
    // printf("BYTES WRITTEN FOR DATA: %d\n", datasent);

    uint8_t fifostatus = nrf24_ReadReg(device, FIFO_STATUS);
    printf("FIFOSTATUS: %x\n", fifostatus);
    if((fifostatus & (1 << 4)) && (!(fifostatus&(1 << 3)))){
        cmdToSend = FLUSH_TX;
        nrfSendCmd(device, cmdToSend);
        return 1;
    }

    return 0;
}

void NRF24_rxMode(NRF24_t *device, uint8_t *addr, uint16_t channel){
    ce_disable(device);

    nrf24_WriteReg(device, RF_CH, channel);
    uint8_t en_rxaddr = nrf24_ReadReg(device, EN_RXADDR);
    en_rxaddr = en_rxaddr | (1 << 1);
    nrf24_WriteReg(device, EN_RXADDR, en_rxaddr);

    nrf24_WriteMultiReg(device, RX_ADDR_P1, addr, 5);

    nrf24_WriteReg(device, RX_PW_P1, 1); // tu było 32

    uint8_t config = nrf24_ReadReg(device, CONFIG);
    config = config | (1 << 1) | (1 << 0);
    nrf24_WriteReg(device, CONFIG, config);


    ce_enable(device);
}

uint8_t isDataAvaliable(NRF24_t *device, int pipenum){
    uint8_t status = nrf24_ReadReg(device, STATUS);
    if((status&(1<<6)) && (status & (pipenum << 1))){
        nrf24_WriteReg(device, STATUS, (1<<6));
        return 1;
    }
    return 0;
}

void NRF24_Receive(NRF24_t *device, uint8_t *data){
    uint8_t cmdtosend = 0;
    cs_select(device);
    cmdtosend = R_RX_PAYLOAD;
    spi_write_blocking(device->spi, &cmdtosend, 1);
    spi_read_blocking(device->spi, 0, data, 1); //tu było 32

    cs_deselect(device);

    sleep_us(300);

    cmdtosend = FLUSH_RX;
    nrfSendCmd(device, cmdtosend);
}
