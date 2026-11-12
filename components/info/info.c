#include <stdio.h>
#include "info.h"

#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_psram.h"
#include "esp_heap_caps.h"

#define TAG "A1_INFO"

void get_info(void)
{
    ESP_LOGI(TAG, "PSRAM size %u bytes", esp_psram_get_size());
    ESP_LOGI(TAG, "Heap free size %u bytes", heap_caps_get_free_size(MALLOC_CAP_8BIT));
    int64_t uptime_s = esp_timer_get_time() * 1e-6;
    ESP_LOGI(TAG, "Time since start up %lu seconds", uptime_s);
    ESP_LOGI(TAG, "FreeRTOS tick rate: %u Hz", CONFIG_FREERTOS_HZ);
}
