#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
// #include "pico/cyw43_arch.h"
#include "time.h"
#include "NRF24_tests.h"
#include "adxl345.h"
#include "hx711_pico.h"

#define NRF24_CE_PIN 20
#define NRF24_CSN_PIN 17

#define SPI_MOSI 19
#define SPI_MISO 16
#define SPI_SCK 18

#define NRF24_SPI spi0

// --- GLOBALNE ZMIENNE DLA CZASU REAKCJI ---
volatile uint64_t t_start_us = 0, t_end_us = 0, t_check_us = 0, t_printf_us = 0, t_start_loop_us = 0;
volatile bool pomiar_aktywny = false;
#define THRESHOLD_G 0.5 // Próg przyspieszenia do wykrycia ruchu (w g)
#define GRAVITY_MSS 9.80665f // Przyspieszenie ziemskie w m/s^2


void cs_select(void){
    gpio_put(NRF24_CSN_PIN, 0);
    sleep_us(10);
}

void cs_deselect(void){
    sleep_us(10);
    gpio_put(NRF24_CSN_PIN, 1);
}

void ce_enable(void){
    gpio_put(NRF24_CE_PIN, 1);
    sleep_us(10);
}

void ce_disable(void){
    sleep_us(10);
    gpio_put(NRF24_CE_PIN, 0);
}

void nrf24_WriteReg(uint8_t reg, uint8_t data){
    uint8_t buf[2];
    buf[0] = reg|1<<5;
    buf[1] = data;

    cs_select();

    spi_write_blocking(NRF24_SPI, buf, 2);

    cs_deselect();

}

void nrf24_WriteMultiReg(uint8_t reg, uint8_t *data, int size){
    uint8_t buf[2];
    buf[0] = reg|1<<5;

    cs_select();

    spi_write_blocking(NRF24_SPI, buf, 2);
    spi_write_blocking(NRF24_SPI, data, size);

    cs_deselect();
}

uint8_t nrf24_ReadReg (uint8_t reg){
    uint8_t data = 0;
    cs_select();

    spi_write_blocking(NRF24_SPI, &reg, 1);
    spi_read_blocking(NRF24_SPI, 0x00, &data, 1);

    cs_deselect();

    return data;
}

void nrf24_ReadMultiReg(uint8_t reg, uint8_t *data, int size){
    cs_select();
    spi_write_blocking(NRF24_SPI, &reg, 1);
    spi_read_blocking(NRF24_SPI, 0, data, size);
    cs_deselect();
}

void nrfSendCmd(uint8_t cmd){
    cs_select();

    spi_write_blocking(NRF24_SPI, &cmd, 1);

    cs_deselect();
}

void NRF24_init(void){

    spi_init(NRF24_SPI, 1000 * 1000);

    gpio_init(NRF24_CE_PIN);
    gpio_init(NRF24_CSN_PIN);

    gpio_set_dir(NRF24_CE_PIN, 1);
    gpio_set_dir(NRF24_CSN_PIN, 1);

    gpio_set_function(SPI_MISO, GPIO_FUNC_SPI);
    gpio_set_function(SPI_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SCK, GPIO_FUNC_SPI);

    ce_disable();

    nrf24_WriteReg(CONFIG, 0);
    nrf24_WriteReg(EN_AA, 0);
    nrf24_WriteReg(EN_RXADDR, 0);
    nrf24_WriteReg(SETUP_AW, 0x03);
    nrf24_WriteReg(SETUP_RETR, 0);
    nrf24_WriteReg(RF_CH, 0);
    nrf24_WriteReg(RF_SETUP, 0x0E);

    ce_enable();
}

void NRF24_txMode(uint8_t *addr, uint16_t channel){
    ce_disable();

    nrf24_WriteReg(RF_CH, channel);
    nrf24_WriteMultiReg(TX_ADDR, addr, 5);
    uint8_t config = nrf24_ReadReg(CONFIG);
    config = config | (1 << 1);
    nrf24_WriteReg(CONFIG, config);


    ce_enable();
}

uint8_t NRF24_transmit(uint8_t *data){
    uint8_t cmdToSend = 0;
   
    cs_select();

    cmdToSend = W_TX_PAYLOAD;
    // uint8_t cmdsent = spi_write_blocking(NRF24_SPI, &cmdToSend, 1);
    // sleep_ms(5);
    spi_write_blocking(NRF24_SPI, &cmdToSend, 1);
    // sleep_ms(5);
    // uint8_t datasent = spi_write_blocking(NRF24_SPI, data, 32);
    spi_write_blocking(NRF24_SPI, data, 1); //tu było 32
    // sleep_ms(5);

    cs_deselect();

    sleep_us(300);

    // printf("BYTES WRITTEN FOR CMD: %d\n", cmdsent);
    // printf("BYTES WRITTEN FOR DATA: %d\n", datasent);

    uint8_t fifostatus = nrf24_ReadReg(FIFO_STATUS);
    printf("FIFOSTATUS: %x\n", fifostatus);
    if((fifostatus & (1 << 4)) && (!(fifostatus&(1 << 3)))){
        cmdToSend = FLUSH_TX;
        nrfSendCmd(cmdToSend);
        return 1;
    }

    return 0;
}

void NRF24_rxMode(uint8_t *addr, uint16_t channel){
    ce_disable();

    nrf24_WriteReg(RF_CH, channel);
    uint8_t en_rxaddr = nrf24_ReadReg(EN_RXADDR);
    en_rxaddr = en_rxaddr | (1 << 1);
    nrf24_WriteReg(EN_RXADDR, en_rxaddr);

    nrf24_WriteMultiReg(RX_ADDR_P1, addr, 5);

    nrf24_WriteReg(RX_PW_P1, 1); // tu było 32

    uint8_t config = nrf24_ReadReg(CONFIG);
    config = config | (1 << 1) | (1 << 0);
    nrf24_WriteReg(CONFIG, config);


    ce_enable();
}

uint8_t isDataAvaliable(int pipenum){
    uint8_t status = nrf24_ReadReg(STATUS);
    if((status&(1<<6)) && (status & (pipenum << 1))){
        nrf24_WriteReg(STATUS, (1<<6));
        return 1;
    }
    return 0;
}

void NRF24_Receive(uint8_t *data){
    uint8_t cmdtosend = 0;
    cs_select();
    cmdtosend = R_RX_PAYLOAD;
    spi_write_blocking(NRF24_SPI, &cmdtosend, 1);
    spi_read_blocking(NRF24_SPI, 0, data, 1); //tu było 32

    cs_deselect();

    sleep_us(300);

    cmdtosend = FLUSH_RX;
    nrfSendCmd(cmdtosend);
}

uint8_t TxAddr[] = {0xEE, 0xDD, 0xCC, 0xBB, 0xAA};
uint8_t TxData[] = "Hello, World\n";
uint8_t RxAddr[] = {0xEE, 0xDD, 0xCC, 0xBB, 0xAA};
uint8_t RxData; //tu było 32

bool led_status = 0;

void toggle_led(){
    if(led_status){
        led_status = false;
    }else{
        led_status = true;
    }
}

bool is_fired = false;

int64_t turn_off(alarm_id_t id, void *user_data){
    gpio_put(5, false);
    return 0;
}

void check_btn(uint8_t event){
    if(event & 0x01){
        if(!is_fired){
            gpio_put(5, true);
            
            add_alarm_in_ms(200, turn_off, NULL, false);

            is_fired = true;
        }
    }else{
        gpio_put(5, false);
    }
    if(event & 0x02){
        is_fired = false;
    }
}

hx711_t hx;

int main()
{
    stdio_init_all();

    // NRF24_init();

    //tx
    // NRF24_txMode(TxAddr, 10);

    //rx
    // NRF24_rxMode(RxAddr, 10);

    // hx711_init(&hx, 15, 14);

    // hx711_tare(&hx, 20);
    // hx711_set_scale(&hx, 0.00123f); // <- ustaw po

    ADXL345_init();

    gpio_init(4);
    gpio_set_dir(4, 0);
    gpio_pull_up(4);
    // gpio_put(4, 1);
    gpio_init(5);
    gpio_set_dir(5, 1);

    // const int16_t THRESHOLD_RAW = (int16_t)(THRESHOLD_G * 256);
    int counter = 0;
    int16_t tab[3600];
    float tab_f[80];
    while (true) {
        //tx
        // if(NRF24_transmit(TxData) == 1){
        //     gpio_put(4, led_status);
        //     toggle_led();
        // }
        // rx

        // if(isDataAvaliable(1) == 1){
        //     NRF24_Receive(&RxData);
        //     printf("%x\n", RxData);
        //     check_btn(RxData);
        // }

        if(!gpio_get(4)){
            check_btn(0x01);
            pomiar_aktywny = true;
            t_start_loop_us = time_us_64();
            counter = 0;
        }

        // --- CZĘŚĆ ADXL345: Pomiar Czasu Reakcji ---
        if (pomiar_aktywny) {
            // add_alarm_in_ms(100, turn_off, NULL, false);
            t_start_us = time_us_64();

            // int32_t raw = hx711_read_raw(&hx);
            int16_t raw_x = ADXL345_read_X_g();
            // float F = (raw - hx.offset) * hx.scale;

            t_end_us = time_us_64();

            if(t_end_us - t_start_us < (1000000 / 3600)){
                sleep_us((1000000 / 3600) - (t_end_us - t_start_us));
            }

            // tab_f[counter] = abs(F);
            tab[counter++] = abs(raw_x);
            
            if(counter == 3600){
                pomiar_aktywny = false;
                t_check_us = time_us_64();
                
                for(uint16_t i = 0; i < 3600; i++){
                    // printf("N: %d X: %d\n", i, tab[i]);
                    printf("%d %d\n", i, tab[i]);
                }
                is_fired = false;
                printf("Czas pomiaru 3600 próbek: %llu us\n", t_check_us - t_start_loop_us);
            }
            
            // if(counter < 1000){
            //     tab[counter++] = raw_x;
            // }else{
            //     sleep_ms(2000);
            //     for(int i = 0; i < 1000; i++){
            //         printf("N: %d X: %d\n", i, tab[i]);
            //     }
            //     pomiar_aktywny = false;
            //     is_fired = false;
            //     t_start_us = 0;
            //     sleep_ms(2000);
            // }
            // Porównanie z progiem (wartość bezwzględna)
            /*
            if (abs(raw_x) > THRESHOLD_RAW) {
                uint64_t t_ruch_us = time_us_64();
                uint64_t RT_us = t_ruch_us - t_start_us;
                float RT_ms = (float)RT_us / 1000.0f;
                
                printf("\n====================================");
                printf("           RUCH WYKRYTY!          ");
                printf("====================================\n");
                printf("Raw X: %d (Próg: > %d)\n", raw_x, THRESHOLD_RAW);
                printf("Czas Startu: %llu us\n", t_start_us);
                printf("Czas Ruchu:  %llu us\n", t_ruch_us);
                printf("--- Czas Reakcji (RT): %.3f ms ---\n", RT_ms);
                printf("====================================\n");
                */
                // Reset
                // pomiar_aktywny = false;
                // is_fired = false;
                // t_start_us = 0;
                
                // Opóźnienie, aby nie mierzyć od razu po ruchu
                // sleep_ms(2000); 
            // }
        }
    }
}
