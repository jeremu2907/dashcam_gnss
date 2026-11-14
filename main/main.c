#include <stdio.h>

#include "components/info/include/info.h"
#include "components/nmea_feed/include/nmea_feed.h"
#include "components/nmea_feed/include/gps_data.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

void app_main(void)
{
    get_info();

    static GpsData gps_data;
    gps_data = create_gps_data();

    xTaskCreate(task_nmea_feed, "periodic", 1000, &gps_data, 1, NULL);
}
