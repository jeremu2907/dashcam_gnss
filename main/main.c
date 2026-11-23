#include <stdio.h>

#include "components/info/include/info.h"
#include "components/nmea_feed/include/nmea_feed.h"
#include "components/nmea_feed/include/gps_data.h"
#include "components/sd_card_iface/include/sd_card_iface.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define SD_STACK_SIZE 1024 * 8
#define NMEA_STACK_SIZE 1024 * 8

void app_main(void)
{
    get_info();

    static GpsData gps_data;
    gps_data = create_gps_data();

    xTaskCreate(task_sd_card, "sd_card", SD_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(task_nmea_feed, "periodic", NMEA_STACK_SIZE, &gps_data, 1, NULL);
}
