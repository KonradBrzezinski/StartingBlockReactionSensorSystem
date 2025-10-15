#include "hardware/i2c.h" // NOWA BIBLIOTEKA DLA I2C
#include <stdio.h>
#include "pico/stdlib.h"

// --- DEFINICJE DLA ADXL345 ---
#define ADXL345_I2C_ADDR 0x53 // Domyślny adres I2C dla ADXL345
#define ADXL345_POWER_CTL 0x2D // Rejestr włączania pomiaru
#define ADXL345_DATA_FORMAT 0x31 // Rejestr formatu danych (zakres)
#define ADXL345_DATAX0 0x32 // Początek rejestrów danych X
#define ADXL345_BW_RATE 0x2C // Rejestr częstotliwości próbkowania

// Wartości Konfiguracyjne
#define ADXL_POWER_MEASURE  0x08 // Ustawia bit 'Measure' (tryb pomiaru)
#define ADXL_RANGE_2G       0x00 // Zakres +/- 2g
#define ADXL_RANGE_4G       0x01 // Zakres +/- 4g
#define ADXL_RANGE_8G       0x02 // Zakres +/- 8g
#define ADXL_RANGE_16G      0x03 // Zakres +/- 16g
#define ADXL_FULL_RES       0x08 // Bit 'FULL_RES' dla pełnej rozdzielczości

// Piny I2C dla ADXL345
#define I2C_PORT i2c0
#define I2C_SDA_PIN 12  // Możesz zmienić na inne wolne piny
#define I2C_SCL_PIN 13  // np. GP4/GP5

typedef struct{
    i2c_inst_t *i2c;
    int sda_pin;
    int scl_pin;
    int i2c_addr;

    /*
        BW RATE:
        00001111 -> 1600Hz ODR: 3200Hz
        00001110 -> 800Hz  ODR: 1600Hz
        00001101 -> 400Hz  ODR: 800Hz
    */
    uint8_t bw_rate;
    
    /*
        DATA_FORMAT:
        00001011 -> +/- 16g
    */
    uint8_t data_format;

    /*
        POWER_CTL:
        0x08 -> 
    */
    uint8_t power_ctl;


    /*
    X, Y, Z
    */
    int16_t data[3];

}ADXL345_t;

void adxl345_write_reg(ADXL345_t *dev, uint8_t reg, uint8_t data);
void adxl345_read_multi(ADXL345_t *dev, uint8_t reg, uint8_t *buf, size_t len);
void ADXL345_init(ADXL345_t *dev);
void ADXL345_read_X_g(ADXL345_t *dev);