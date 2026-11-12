#include <stdlib.h>
#include <string.h>

#include "nmea_helper.h"

#define NMEA_ID_SIZE 3

void get_nmea_id(const char* nmea, char* id)
{
    size_t i_nmea = NMEA_ID_SIZE,
                 i_id = 0;
    while(true)
    {
        if(nmea[i_nmea] == ',' ||
           nmea[i_nmea] == '\0')
        {
            return;
        }

        id[i_id] = nmea[i_nmea];

        i_id++;
        i_nmea++;
    }
}

bool is_nmea_rmc(const char* nmea)
{
    char *id = calloc(NMEA_ID_SIZE + 1, sizeof(char));
    get_nmea_id(nmea, id);

    bool comp = strcmp(id, "RMC") == 0;
    free(id);
    return comp;
}

bool is_nmea_gga(const char* nmea)
{
    char *id = calloc(NMEA_ID_SIZE + 1, sizeof(char));
    get_nmea_id(nmea, id);

    bool comp = strcmp(id, "GGA") == 0;
    free(id);
    return comp;
}