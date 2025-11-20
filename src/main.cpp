#include <stdio.h>
#include "pico/stdlib.h"

#include "FreeRTOS.h"
#include "task.h"

/* GPIO pins */
#define LED_DEFAULT   7   // Should be GPIO 7 on Pico 2
#define LED_OTHER     24
#define PACK_PARAMS(pin, on_ms)   (((on_ms & 0xFFFF) << 16) | (pin & 0xFFFF))
#define UNPACK_PIN(x)             ((x) & 0xFFFF)
#define UNPACK_ONMS(x)            (((x) >> 16) & 0xFFFF)
/* Task: Blink default LED */
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


/* Hardware Setup */
void prvSetupHardware(void)
{
    stdio_init_all();
}

int main()
{
    prvSetupHardware();
    gpio_put(LED_DEFAULT, 1);
    /* Create tasks */
    if (xTaskCreate(blinkDefaultLED, "Blink7", 1024, NULL, 1, NULL) != pdPASS) {
        printf("Failed to create Blink7\n");
        return 1;
    }

   
   xTaskCreate(blinkTask, "Blink3", 1024, (void*)PACK_PARAMS(3, 250), 1, NULL);
   xTaskCreate(blinkTask, "Blink5", 1024, (void*)PACK_PARAMS(5, 500), 1, NULL);
   xTaskCreate(blinkTask, "Blink6", 1024, (void*)PACK_PARAMS(6, 1000), 1, NULL);
   xTaskCreate(blinkTask, "Blink9", 1024, (void*)PACK_PARAMS(9, 2000), 1, NULL);
   xTaskCreate(blinkTask, "Blink24", 1024, (void*)PACK_PARAMS(24, 4000), 1, NULL);

    /* Start FreeRTOS scheduler (never returns) */
    vTaskStartScheduler();

    while (true) { }
    return 0;
}
