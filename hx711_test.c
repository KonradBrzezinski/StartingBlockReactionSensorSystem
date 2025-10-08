#include <stdio.h>
#include "pico/stdlib.h"
#include "hx711_pico.h"

#define HX_DOUT  15
#define HX_SCK   14

hx711_t hx;

int main() {
    stdio_init_all();
    hx711_init(&hx, HX_DOUT, HX_SCK);

    // Zerowanie
    hx711_tare(&hx, 20);
    hx711_set_scale(&hx, 0.00123f); // <- ustaw po kalibracji

    const uint32_t interval_us = 12500; // 1 / 80 Hz = 12.5 ms
    absolute_time_t next = get_absolute_time();

    while (true) {
        // czekaj do następnej próbki
        sleep_until(next);
        next = delayed_by_us(next, interval_us);

        // odczyt jednej próbki
        int32_t raw = hx711_read_raw(&hx);
        float F = (raw - hx.offset) * hx.scale;

        printf("Siła: %.2f N\n", F);
    }
}
