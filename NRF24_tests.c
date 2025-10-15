#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
// #include "pico/cyw43_arch.h"
#include "time.h"
#include "includes/NRF24_tests.h"
#include "includes/adxl345.h"
#include "includes/hx711_pico.h"
#include "includes/nrf24l01.h"

// --- GLOBALNE ZMIENNE DLA CZASU REAKCJI ---
volatile uint64_t t_start_us = 0, t_end_us = 0, t_check_us = 0, t_printf_us = 0, t_start_loop_us = 0;
volatile bool pomiar_aktywny = false;
#define THRESHOLD_G 0.5 // Próg przyspieszenia do wykrycia ruchu (w g)
#define GRAVITY_MSS 9.80665f // Przyspieszenie ziemskie w m/s^2

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

ADXL345_t adxl345;

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
