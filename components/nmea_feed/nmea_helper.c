#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "esp_log.h"

#include "nmea_helper.h"

#define NMEA_ID_SIZE 3
#define TAG "nmea_helper"
#define MINUTES_PER_DEGREE 60.0

void get_nmea_id(const char *nmea, char *id)
{
    size_t i_nmea = NMEA_ID_SIZE,
           i_id = 0;
    while (true)
    {
        if (nmea[i_nmea] == ',' ||
            nmea[i_nmea] == '\0')
        {
            return;
        }

        id[i_id] = nmea[i_nmea];

        i_id++;
        i_nmea++;
    }
}

bool is_nmea_rmc(const char *nmea)
{
    char *id = calloc(NMEA_ID_SIZE + 1, sizeof(char));
    get_nmea_id(nmea, id);

    bool comp = strcmp(id, "RMC") == 0;
    free(id);
    return comp;
}

bool is_nmea_gga(const char *nmea)
{
    char *id = calloc(NMEA_ID_SIZE + 1, sizeof(char));
    get_nmea_id(nmea, id);

    bool comp = strcmp(id, "GGA") == 0;
    free(id);
    return comp;
}

float parse_epoch_s(char *const token)
{
    float epoch_s = 0.0f;

    char *hh = calloc(3, sizeof(char));
    strncpy(hh, token, 2);
    epoch_s += strtof(hh, NULL) * 3600.0f;
    free(hh);
    if (errno == ERANGE)
    {
        ESP_LOGE(TAG, "Could not parse utc hh");
    }

    char *mm = calloc(3, sizeof(char));
    strncpy(mm, token + 2, 2);
    epoch_s += strtof(mm, NULL) * 60.0f;
    free(mm);
    if (errno == ERANGE)
    {
        ESP_LOGE(TAG, "Could not parse utc mm");
    }

    epoch_s += strtof(token + 4, NULL);
    if (errno == ERANGE)
    {
        ESP_LOGE(TAG, "Could not parse utc ss");
    }

    return epoch_s;
}

float parse_lat_deg(char *const token)
{
    float lat = 0.0f;

    size_t lat_dd_len = 2;
    char *lat_dd = calloc(lat_dd_len + 1, sizeof(char));
    strncpy(lat_dd, token, lat_dd_len);
    lat = strtof(lat_dd, NULL);
    free(lat_dd);
    if (errno == ERANGE)
    {
        ESP_LOGE(TAG, "Could not parse lat_dd");
    }
    lat += strtof(token + lat_dd_len, NULL) / MINUTES_PER_DEGREE;
    if (errno == ERANGE)
    {
        ESP_LOGE(TAG, "Could not parse lat_m");
    }

    return lat;
}

float parse_lon_deg(char *const token)
{
    float lon = 0.0f;

    size_t lon_ddd_len = 3;
    char *lon_ddd = calloc(lon_ddd_len + 1, sizeof(char));
    strncpy(lon_ddd, token, lon_ddd_len);
    lon = strtof(lon_ddd, NULL);
    free(lon_ddd);
    if (errno == ERANGE)
    {
        ESP_LOGE(TAG, "Could not parse lon_ddd");
    }
    lon += strtof(token + lon_ddd_len, NULL) / MINUTES_PER_DEGREE;
    if (errno == ERANGE)
    {
        ESP_LOGE(TAG, "Could not parse lon_m");
    }

    return lon;
}

float parse_lat_dir(char *const token)
{
    if (strlen(token) != 1)
    {
        ESP_LOGE(TAG, "Invalid direction strlen");
    }
    switch (token[0])
    {
    case 'N':
        return 1.0;

    case 'S':
        return -1.0;

    default:
        return 0.0;
        ESP_LOGE(TAG, "Invalid %s direction", token);
    }
}

float parse_lon_dir(char *const token)
{
    if (strlen(token) != 1)
    {
        ESP_LOGE(TAG, "Invalid direction strlen");
    }
    switch (token[0])
    {
    case 'E':
        return 1.0;

    case 'W':
        return -1.0;

    default:
        return 0.0;
        ESP_LOGE(TAG, "Invalid %s direction", token);
    }
}

float parse_float(char *const token, char *name)
{
    float val = strtof(token, NULL);
    if (errno == ERANGE)
    {
        ESP_LOGE(TAG, "Could not parse %s", name);
    }

    return val;
}

void parse_yymmdd(char *const token, short fields[3])
{
    size_t field_size = 3;

    for(size_t i = 0; i < field_size; i++)
    {
        char *ff = calloc(field_size, sizeof(char));
        strncpy(ff, token + 2 * i, field_size - 1);
        fields[i] = strtof(ff, NULL);
        free(ff);
        if (errno == ERANGE)
        {
            ESP_LOGE(TAG, "Could not parse yymmdd");
        }
    }
}

void parse_gga(char *const *tokens, size_t size, GpsData *gps_data)
{
    if (size < 6)
    {
        ESP_LOGE(TAG, "Invalid number of gga tokens");
        return;
    }

    size_t utc_idx = 1;
    gps_data->utc = parse_epoch_s(tokens[utc_idx]);

    size_t lat_idx = 2;
    gps_data->lat = parse_lat_deg(tokens[lat_idx]);

    size_t lat_dir_idx = 3;
    gps_data->lat *= parse_lat_dir(tokens[lat_dir_idx]);

    size_t lon_idx = 4;
    gps_data->lon = parse_lon_deg(tokens[lon_idx]);

    size_t lon_dir_idx = 5;
    gps_data->lon *= parse_lon_dir(tokens[lon_dir_idx]);
}

void parse_rmc(char *const *tokens, size_t size, GpsData *gps_data)
{
    if (size < 6)
    {
        ESP_LOGE(TAG, "Invalid number of rmc tokens");
        return;
    }

    size_t utc_idx = 1;
    gps_data->utc = parse_epoch_s(tokens[utc_idx]);

    size_t lat_idx = 3;
    gps_data->lat = parse_lat_deg(tokens[lat_idx]);

    size_t lat_dir_idx = 4;
    gps_data->lat *= parse_lat_dir(tokens[lat_dir_idx]);

    size_t lon_idx = 5;
    gps_data->lon = parse_lon_deg(tokens[lon_idx]);

    size_t lon_dir_idx = 6;
    gps_data->lon *= parse_lon_dir(tokens[lon_dir_idx]);

    size_t speed_idx = 7;
    gps_data->speed = parse_float(tokens[speed_idx], "speed");

    size_t heading_idx = 8;
    gps_data->heading = parse_float(tokens[heading_idx], "heading");

    size_t ddmmyy_idx = 9;
    short ddmmyy[3] = {0};
    parse_yymmdd(tokens[ddmmyy_idx], ddmmyy);
    gps_data->day = ddmmyy[0];
    gps_data->month = ddmmyy[1];
    gps_data->year = ddmmyy[2];
}