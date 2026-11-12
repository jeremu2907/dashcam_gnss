#pragma once

#include "gps_data.h"

void get_nmea_id(const char* nmea, char* id);
bool is_nmea_rmc(const char* nmea);
bool is_nmea_gga(const char* nmea);

void parse_gga(char* const* tokens, size_t size, GpsData *gps_data);
void parse_rmc(char* const* tokens, size_t size, GpsData *gps_data);