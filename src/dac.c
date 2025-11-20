#include "dac.h"

static const uint8_t dac_pins[8] = {
    3,  // D0 (LSB)
    5, // D1
    6, // D2
    9,  // D3
    10,  // D4
    11,  // D5
    4,  // D6
    7   // D7 (MSB)
};


void dac_init(void) {
    for (int i = 0; i < 8; i++) {
        gpio_init(dac_pins[i]);
        gpio_set_dir(dac_pins[i], GPIO_OUT);
        gpio_put(dac_pins[i], 0);
    }
}

void dac_write(uint8_t value) {
    for (int bit = 0; bit < 8; bit++) {
        gpio_put(dac_pins[bit], (value >> bit) & 1);
    }
}
