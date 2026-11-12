#include "esp_log.h"
#include <math.h>

#include "gps_data.h"

#define SENTINEL_FLOAT -HUGE_VALF
#define SENTINEL_SHORT -HUGE_VAL
#define TAG "gps_data"

GpsData create_gps_data()
{
    GpsData gps_data = {
        .utc = SENTINEL_FLOAT,
        .lat = SENTINEL_FLOAT,
        .lon = SENTINEL_FLOAT,
        .speed = SENTINEL_FLOAT,
        .heading = SENTINEL_FLOAT,
        .day = SENTINEL_SHORT,
        .month = SENTINEL_SHORT,
        .year = SENTINEL_SHORT,
    };

    return gps_data;
}

void log_info_gps_data(const GpsData *g)
{
    ESP_LOGI(TAG, "\n\
        utc: %fs\n\
        lat: %fdeg\n\
        lon: %fdeg\n\
        spd: %fkts\n\
        hea: %fdeg T\n\
         dd: %d\n\
         mm: %d\n\
         yy: %d\n\
        ",
        g->utc,
        g->lat,
        g->lon,
        g->speed,
        g->heading,
        g->day,
        g->month,
        g->year
    );
}