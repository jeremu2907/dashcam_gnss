#include <stdio.h>

#include "components/info/include/info.h"
#include "components/nmea_feed/include/nmea_feed.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

typedef struct {
    bool b;
    size_t i;
} task_param;

void taskBounded(void* pvParameters)
{
    // Number of ticks
    const TickType_t xDelay = 500 / portTICK_PERIOD_MS;

    for(int i = 0; i < 10; i++)
    {
        task_param *tp = (task_param *)pvParameters;
        tp->b = !tp->b;
        tp->i++;

        vTaskDelay(xDelay);
    }

    vTaskDelete(NULL);
}

void app_main(void)
{
    const char TAG[] = "app_main";
    get_info();

    task_param tp = {
        false,
        1
    };
    xTaskCreate(taskBounded, "bounded", 1000, (void*)&tp, 1, NULL);
    xTaskCreate(task_nmea_feed, "periodic", 1000, nullptr, 1, NULL);
}
