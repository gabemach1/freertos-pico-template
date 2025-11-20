#include <stdio.h>
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"
#include "dac.h"

/* GPIO pins */
#define LED_DEFAULT   7   // Should be GPIO 7 on Pico 2
#define LED_OTHER     24
#define PACK_PARAMS(pin, on_ms)   (((on_ms & 0xFFFF) << 16) | (pin & 0xFFFF))
#define UNPACK_PIN(x)             ((x) & 0xFFFF)
#define UNPACK_ONMS(x)            (((x) >> 16) & 0xFFFF)
/* Task: Blink default LED */

/* 256-sample sine wave lookup table (0–255 output) */
const uint16_t sineTable[] = {
128, 131, 134, 137, 140, 143, 146, 149,
152, 155, 158, 162, 165, 167, 170, 173,
176, 179, 182, 185, 188, 190, 193, 196,
198, 201, 203, 206, 208, 211, 213, 215,
218, 220, 222, 224, 226, 228, 230, 232,
234, 235, 237, 238, 240, 241, 243, 244,
245, 246, 248, 249, 250, 250, 251, 252,
253, 253, 254, 254, 254, 255, 255, 255,
255, 255, 255, 255, 254, 254, 254, 253,
253, 252, 251, 250, 250, 249, 248, 246,
245, 244, 243, 241, 240, 238, 237, 235,
234, 232, 230, 228, 226, 224, 222, 220,
218, 215, 213, 211, 208, 206, 203, 201,
198, 196, 193, 190, 188, 185, 182, 179,
176, 173, 170, 167, 165, 162, 158, 155,
152, 149, 146, 143, 140, 137, 134, 131,
128, 124, 121, 118, 115, 112, 109, 106,
103, 100, 97, 93, 90, 88, 85, 82,
79, 76, 73, 70, 67, 65, 62, 59,
57, 54, 52, 49, 47, 44, 42, 40,
37, 35, 33, 31, 29, 27, 25, 23,
21, 20, 18, 17, 15, 14, 12, 11,
10, 9, 7, 6, 5, 5, 4, 3,
2, 2, 1, 1, 1, 0, 0, 0,
0, 0, 0, 0, 1, 1, 1, 2,
2, 3, 4, 5, 5, 6, 7, 9,
10, 11, 12, 14, 15, 17, 18, 20,
21, 23, 25, 27, 29, 31, 33, 35,
37, 40, 42, 44, 47, 49, 52, 54,
57, 59, 62, 65, 67, 70, 73, 76,
79, 82, 85, 88, 90, 93, 97, 100,
103, 106, 109, 112, 115, 118, 121, 124};


void blinkDefaultLED(void *pvParameters)
{
    gpio_init(LED_DEFAULT);
    gpio_set_dir(LED_DEFAULT, GPIO_OUT);

    while (true) {
        gpio_put(LED_DEFAULT, 1);
        vTaskDelay(pdMS_TO_TICKS(500));
        gpio_put(LED_DEFAULT, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void blinkTask(void *pvParameters)
{
    uint32_t packed = (uint32_t)pvParameters;

    uint pin = UNPACK_PIN(packed);
    uint on_ms = UNPACK_ONMS(packed);

    // fixed off-time or could compute duty here
    uint off_ms = on_ms;

    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);

    while (true) {
        gpio_put(pin, 1);
        vTaskDelay(pdMS_TO_TICKS(on_ms));
        
        gpio_put(pin, 0);
        vTaskDelay(pdMS_TO_TICKS(off_ms));
    }
}
uint8_t voltage_to_byte(float volts) {
    if (volts < 0) volts = 0;
    if (volts > 3.3f) volts = 3.3f;
    float scale = volts / 3.3f;
    return (uint8_t)(scale * 255.0f);
}
void sawTask(void *pvParameters)
{
    const int max = 256;          // 8-bit range
    const int delay_us = 50;      // adjust frequency (smaller = faster wave)

    while (true)
    {
        for (uint16_t v = 0; v < max; v++) {
            dac_write((uint8_t)v);
            sleep_us(delay_us);
        }
    }
}
void sineTask(void *pvParameters)
{
    const int freq = 10000;               // desired output frequency (Hz)
    const int tableSize = 256;
    const int sampleRate = freq * tableSize;

    // Compute delay per sample in microseconds
    int delay_us = 1000000 / sampleRate;

    int index = 0;

    while (1)
    {
        dac_write(sineTable[index]);

        index++;
        if (index >= tableSize)
            index = 0;

        //sleep_us(delay_us);
    }
}

/* Hardware Setup */
void prvSetupHardware(void)
{
    stdio_init_all();
}

int main()
{
    prvSetupHardware();
    dac_init();
    printf("Launching FreeRTOS tasks...\n");

    /* Create tasks */
    if (xTaskCreate(blinkDefaultLED, "Blink7", 1024, NULL, 1, NULL) != pdPASS) {
        printf("Failed to create Blink7\n");
        return 1;
    }
    gpio_init(LED_DEFAULT);
    gpio_set_dir(LED_DEFAULT, GPIO_OUT);
    gpio_put(LED_DEFAULT, 1);
   /*
   xTaskCreate(blinkTask, "Blink3", 1024, (void*)PACK_PARAMS(3, 250), 1, NULL);
   xTaskCreate(blinkTask, "Blink5", 1024, (void*)PACK_PARAMS(5, 500), 1, NULL);
   xTaskCreate(blinkTask, "Blink6", 1024, (void*)PACK_PARAMS(6, 1000), 1, NULL);
   xTaskCreate(blinkTask, "Blink9", 1024, (void*)PACK_PARAMS(9, 2000), 1, NULL);
   xTaskCreate(blinkTask, "Blink24", 1024, (void*)PACK_PARAMS(24, 4000), 1, NULL);
   */
   //xTaskCreate(dacTask, "DAC", 1024, NULL, 2, NULL);
        // *** HARD CODED VOLTAGE ***
    xTaskCreate(sineTask, "SINE", 1024, NULL, 2, NULL);


    /* Start FreeRTOS scheduler (never returns) */
    vTaskStartScheduler();

    while (true) { }
    return 0;
}
