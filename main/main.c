#include <stdio.h>

#include "components/info/include/info.h"
#include "components/nmea_feed/include/nmea_feed.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

void app_main(void)
{
    get_info();

    xTaskCreate(task_nmea_feed, "periodic", 1000, nullptr, 1, NULL);
}
