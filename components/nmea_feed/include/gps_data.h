#pragma once

typedef struct gps_data
{
    float utc,
        lat,
        lon,
        speed,
        heading;

    short day,
        month,
        year;
} GpsData;

GpsData create_gps_data();

void log_info_gps_data(const GpsData *g);
