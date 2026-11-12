#pragma once

typedef struct gps_data_t
{
    double utc,
           lat,
           lon,
           speed,
           heading;

    short day,
          month;

    int year;
}
