#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "gps_data.h"
#include "nmea_feed.h"
#include "nmea_helper.h"
#include "nmea_sentences.h"
#include "utils/include/string_utils.h"
#include "sd_card_iface/include/sd_card_iface.h"

#define TAG "nmea_feed"
#define CHECKSUM_SPLIT_FIELDS 2
#define MAX_NMEA_FIELDS 32

static const size_t NMEA_SIZE = sizeof(nmea_sentences) / sizeof(nmea_sentences[0]);

void task_nmea_feed(void *pvParameters)
{
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;

    GpsData *gps_data = (GpsData*)pvParameters;

    char **sentence_checksum_buff = calloc(CHECKSUM_SPLIT_FIELDS, sizeof(char *));
    char **tokens_buff = calloc(MAX_NMEA_FIELDS, sizeof(char *));

    size_t i = 0;
    while (true)
    {
        const char *nmea = nmea_sentences[i];
        const bool is_gga = is_nmea_gga(nmea);
        const bool is_rmc = is_nmea_rmc(nmea);
        if (is_gga || is_rmc)
        {
            if (split('*', nmea, sentence_checksum_buff, CHECKSUM_SPLIT_FIELDS) != 0)
            {
                vTaskDelay(xDelay);
                continue;
            }
            ESP_LOGI(TAG, "\nSentence: %s\nChecksum: %s", sentence_checksum_buff[0], sentence_checksum_buff[CHECKSUM_SPLIT_FIELDS - 1]);

            const char *nmea_fields_str = sentence_checksum_buff[0];
            size_t tokens_size = count_char(',', nmea_fields_str) + 1;

            if (split(',', nmea_fields_str, tokens_buff, tokens_size) != 0)
            {
                vTaskDelay(xDelay);
                continue;
            }

            if (is_gga)
            {
                parse_gga(tokens_buff, tokens_size, gps_data);
            }
            else if(is_rmc)
            {
                parse_rmc(tokens_buff, tokens_size, gps_data);
            }
            // log_info_gps_data(gps_data);

            size_t gps_data_str_size = 256;
            char gps_data_str[gps_data_str_size];
            int buf_size = gps_data_to_string(gps_data, gps_data_str, gps_data_str_size);
            gps_data_str[buf_size] = '\n';
            gps_data_str[buf_size + 1] = '\0';
            ESP_LOGI(TAG, "%s", gps_data_str);
            enqueue_gps(gps_data_str, buf_size + 2);

            vTaskDelay(xDelay);
        }
        i++;
        i %= NMEA_SIZE;
    }

    free(sentence_checksum_buff);
    free(tokens_buff);

    vTaskDelete(NULL);
}
