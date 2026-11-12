#pragma once

void get_nmea_id(const char* nmea, char* id);
bool is_nmea_rmc(const char* nmea);
bool is_nmea_gga(const char* nmea);