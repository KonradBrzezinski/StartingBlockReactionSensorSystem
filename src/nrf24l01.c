#include "nrf24l01.h"

void csn_select(void){
    gpio_put(SPI_CSN_PIN, 0);
    sleep_us(3);
}

void csn_deselect(void){
    sleep_us(3);
    gpio_put(SPI_CSN_PIN, 1);
}

/*
Writing register to nrf24 module
*/
void nrf24_write_reg(spi_inst_t *spi, uint8_t reg, uint8_t value){
    uint8_t buf[2];
    buf[0] = (W_REGISTER | reg);
    buf[1] = value;
    csn_select();
    spi_write_blocking(spi, buf, 2);
    csn_deselect();
}

uint8_t nrf24_read_reg(spi_inst_t *spi, uint8_t reg){
    uint8_t cmd = (R_REGISTER | reg);
    uint8_t value;
    csn_select();
    spi_write_blocking(spi, &cmd, 1);
    spi_read_blocking(spi, 0x00, &value, 1);
    csn_deselect();
    return value;
}

void nrf24_init(spi_inst_t *spi, uint32_t baudrate){

    spi_init(spi, baudrate);

    spi_set_format(spi, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_RX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_TX_PIN, GPIO_FUNC_SPI);
    // gpio_set_function(SPI_CSN_PIN, GPIO_FUNC_SPI);

    gpio_init(SPI_CSN_PIN);
    gpio_init(NRF24_CE_PIN);

    gpio_set_dir(SPI_CSN_PIN, GPIO_OUT);
    gpio_set_dir(NRF24_CE_PIN, GPIO_OUT);

    gpio_put(SPI_CSN_PIN, 1);


    nrf24_write_reg(spi, SETUP_AW, 0x03); // 5 bytes address
}

void nrf24_init_tx(spi_inst_t *spi){
    gpio_put(NRF24_CE_PIN, 0);
    
    nrf24_write_reg(spi, 0x00, 0x0E);

    nrf24_write_reg(spi, RX_PW_P0, 32);

    uint8_t addr[] = {0xE7, 0xE7, 0xE7, 0xE7, 0xE7};

    static const uint8_t cmd_tx = W_REGISTER | TX_ADDR;

    csn_select();
    spi_write_blocking(spi, &cmd_tx, 1);
    spi_write_blocking(spi, addr, 5);
    csn_deselect();

    static const uint8_t cmd_rx = W_REGISTER | RX_ADDR_P0;

    csn_select();
    spi_write_blocking(spi, &cmd_rx, 1);
    spi_write_blocking(spi, addr, 5);
    csn_deselect();

    nrf24_write_reg(spi, 0x01, 0x3F);
    nrf24_write_reg(spi, 0x02, 0x01);
    nrf24_write_reg(spi, 0x03, 0x03);
    nrf24_write_reg(spi, 0x04, 0x04);
    nrf24_write_reg(spi, 0x05, 0x1E);
    nrf24_write_reg(spi, 0x06, 0x0E);
    sleep_ms(5);
}

void nrf24_init_rx(spi_inst_t *spi){
    gpio_put(NRF24_CE_PIN, 0);
    nrf24_write_reg(spi, 0x00, 0x0F);
    nrf24_write_reg(spi, 0x01, 0x3F);
    nrf24_write_reg(spi, 0x02, 0x01);
    nrf24_write_reg(spi, 0x03, 0x03);
    nrf24_write_reg(spi, 0x04, 0x04);
    nrf24_write_reg(spi, 0x05, 0x1E);
    nrf24_write_reg(spi, 0x06, 0x07);
    sleep_ms(5);
    gpio_put(NRF24_CE_PIN, 1);
}

void nrf24_write_payload(spi_inst_t *spi, uint8_t *data, int len){
    uint8_t cmd = W_TX_PAYLOAD;
    csn_select();
    spi_write_blocking(spi, &cmd, 1);
    spi_write_blocking(spi, data, len);
    csn_deselect();
}

void nrf24_read_payload(spi_inst_t *spi, uint8_t *data, int len){
    uint8_t cmd = R_RX_PAYLOAD;
    csn_select();
    spi_write_blocking(spi, &cmd, 1);
    spi_read_blocking(spi, 0x00, data, len);
    csn_deselect();
}

void send_msg(spi_inst_t *spi, char *msg){

    nrf24_write_reg(spi, STATUS, (1 << 5) | (1 << 4) | (1 << 6));

    gpio_put(NRF24_CE_PIN, 0);

    //5 -> TX_DS, 4 -> MAX_RT
    nrf24_write_reg(spi, STATUS, (1 << 5) | (1 << 4));

    nrf24_write_payload(spi, (uint8_t*)msg, strlen(msg));

    gpio_put(NRF24_CE_PIN, 1);
    sleep_us(20);
    gpio_put(NRF24_CE_PIN, 0);

    uint8_t status = nrf24_read_reg(spi, STATUS);
    printf("Status po próbie nadania: 0x%X\n", status);

    uint8_t observe_tx = nrf24_read_reg(spi, OBSERVE_TX);
    printf("OBSERVE_TX po próbie nadania: 0x%X\n", observe_tx);

    printf("Sent message: %s\n", msg);
}

void receive_msg(spi_inst_t *spi){
    uint8_t status = nrf24_read_reg(spi, STATUS);
    if(status & (1 << 6)){
        uint8_t rxbuf[32];
        nrf24_read_payload(spi, rxbuf, 32);
        printf("Received: %s\n", rxbuf);
        nrf24_write_reg(spi, STATUS, (1 << 6));
    }
}