#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "gps_data.h"
#include "nmea_feed.h"
#include "nmea_helper.h"
#include "nmea_sentences.h"
#include "utils/include/string_utils.h"

#define TAG "nmea_feed"

static const size_t NMEA_SIZE = sizeof(nmea_sentences) / sizeof(nmea_sentences[0]);

void task_nmea_feed(void *pvParameters)
{
    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;

    GpsData gps_data = create_gps_data();

    size_t i = 0;
    while (true)
    {
        const char *nmea = nmea_sentences[i];
        const bool is_gga = is_nmea_gga(nmea);
        const bool is_rmc = is_nmea_rmc(nmea);
        if (is_gga || is_rmc)
        {
            size_t sentence_checksum_size = count_char('*', nmea) + 1;
            char **sentence_checksum = calloc(sentence_checksum_size, sizeof(char *));

            if (split('*', nmea, sentence_checksum, sentence_checksum_size) != 0)
            {
                vTaskDelay(xDelay);
                free(sentence_checksum);
            }
            ESP_LOGI(TAG, "\nSentence: %s\nChecksum: %s", sentence_checksum[0], sentence_checksum[sentence_checksum_size - 1]);

            const char *nmea_fields_str = sentence_checksum[0];
            size_t tokens_size = count_char(',', nmea_fields_str) + 1;
            char **tokens = calloc(tokens_size, sizeof(char *));

            if (split(',', nmea_fields_str, tokens, tokens_size) != 0)
            {
                vTaskDelay(xDelay);
                free(tokens);
                free(sentence_checksum);
            }

            if (is_gga)
            {
                parse_gga(tokens, tokens_size, &gps_data);
            }
            else if(is_rmc)
            {
                parse_rmc(tokens, tokens_size, &gps_data);
            }
            log_info_gps_data(&gps_data);

            vTaskDelay(xDelay);
            free(tokens);
            free(sentence_checksum);
        }
        i++;
        i %= NMEA_SIZE;
    }
}
