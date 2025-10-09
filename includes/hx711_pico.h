// hx711_pico.h
#ifndef HX711_PICO_H
#define HX711_PICO_H

#include "pico/stdlib.h"

typedef struct {
    uint dout;     // pin HX711 DOUT
    uint sck;      // pin HX711 SCK
    int32_t offset; // zerowanie
    float scale;    // przelicznik na Newtony
} hx711_t;

void hx711_init(hx711_t *hx, uint dout, uint sck);
int32_t hx711_read_raw(hx711_t *hx);
void hx711_tare(hx711_t *hx, int samples);
void hx711_set_scale(hx711_t *hx, float scale);
float hx711_get_force(hx711_t *hx, int samples);

#endif
