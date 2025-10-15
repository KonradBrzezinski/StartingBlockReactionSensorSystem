#include "adxl345.h"
// --- FUNKCJE I2C DLA ADXL345 ---

void adxl345_write_reg(ADXL345_t *dev, uint8_t reg, uint8_t data) {
    uint8_t buf[2] = {reg, data};
    i2c_write_blocking(dev->i2c, dev->i2c_addr, buf, 2, false);
}

void adxl345_read_multi(ADXL345_t *dev, uint8_t reg, uint8_t *buf, size_t len) {
    i2c_write_blocking(dev->i2c, dev->i2c_addr, &reg, 1, true); // True to keep I2C open
    i2c_read_blocking(dev->i2c, dev->i2c_addr, buf, len, false);
}

void ADXL345_init(ADXL345_t *dev) {
    // Inicjalizacja I2C
    i2c_init(dev->i2c, 400 * 1000); // 400 kHz
    gpio_set_function(dev->sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(dev->scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(dev->sda_pin);
    gpio_pull_up(dev->scl_pin);

    // Konfiguracja ADXL345:
    
    // 1. Ustawienie Data Rate na 3200 Hz (0x0F)
    adxl345_write_reg(dev, ADXL345_BW_RATE, dev->bw_rate);
    
    // 2. Ustawienie formatu: Pełna rozdzielczość, +/- 16g (0x0B)
    adxl345_write_reg(dev, ADXL345_DATA_FORMAT, 0b00001011);

    // 3. Uruchomienie trybu pomiaru (0x08)
    adxl345_write_reg(dev, ADXL345_POWER_CTL, 0x08);

    printf("ADXL345 zainicjalizowany.\n");
}

void ADXL345_read_X_g(ADXL345_t *dev) {
    uint8_t buffer[6];
    
    // Odczyt X-axis: DATAX0 (0x32) i DATAX1 (0x33)
    adxl345_read_multi(dev, ADXL345_DATAX0, buffer, 6);
    
    // Łączenie bajtów LSB (buffer[0]) i MSB (buffer[1])
    dev->data[0] = (int16_t)(buffer[0] | (buffer[1] << 8)); // X
    dev->data[1] = (int16_t)(buffer[2] | (buffer[3] << 8)); // Y
    dev->data[2] = (int16_t)(buffer[4] | (buffer[5] << 8)); // Z
}
