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
        utc: %11.3f s\n\
        lat: %11.6f deg\n\
        lon: %11.6f deg\n\
        spd: %11.3f kts\n\
        hea: %11.3f deg T\n\
         dd: %11d\n\
         mm: %11d\n\
         yy: %11d\n\
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