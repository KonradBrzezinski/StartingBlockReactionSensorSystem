// hx711_pico.c
#include "hx711_pico.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"

void hx711_init(hx711_t *hx, uint dout, uint sck) {
    hx->dout = dout;
    hx->sck = sck;
    hx->offset = 0;
    hx->scale = 1.0f;

    gpio_init(hx->dout);
    gpio_set_dir(hx->dout, false);

    gpio_init(hx->sck);
    gpio_set_dir(hx->sck, true);
    gpio_put(hx->sck, 0);
}

int32_t hx711_read_raw(hx711_t *hx) {
    // czekaj aż HX711 zgłosi gotowość (DOUT = 0)
    while (gpio_get(hx->dout));

    int32_t value = 0;
    for (int i = 0; i < 24; i++) {
        gpio_put(hx->sck, 1);
        sleep_us(1);
        value = (value << 1) | gpio_get(hx->dout);
        gpio_put(hx->sck, 0);
        sleep_us(1);
    }

    // dodatkowe pulsy = wybór wzmocnienia, domyślnie 128
    gpio_put(hx->sck, 1);
    sleep_us(1);
    gpio_put(hx->sck, 0);
    sleep_us(1);

    // konwersja wartości ze znakiem (24 bity -> 32 bity)
    if (value & 0x800000) {
        value |= ~0xFFFFFF;
    }

    return value;
}

void hx711_tare(hx711_t *hx, int samples) {
    int64_t sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += hx711_read_raw(hx);
    }
    hx->offset = sum / samples;
}

void hx711_set_scale(hx711_t *hx, float scale) {
    hx->scale = scale;
}

float hx711_get_force(hx711_t *hx, int samples) {
    int64_t sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += hx711_read_raw(hx);
    }
    int32_t avg = sum / samples;
    return (avg - hx->offset) * hx->scale;
}
