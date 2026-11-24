#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "esp_err.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"

#include "sd_pwr_ctrl_by_on_chip_ldo.h"

#include "sd_card_iface.h"

#define TAG "sd_card_iface"
#define SD_MOUNT_PATH "/sdcard"
#define FILE_MODE "a"

#define QUEUE_MIN_ITEMS_TO_WRITE 5
#define QUEUE_GPS_LENGTH 64
#define QUEUE_TIMEOUT 5000 / portTICK_PERIOD_MS

#define GPS_CSV_ROW_MAX_LENGTH 256

static char *s_gps_csv_log_path = SD_MOUNT_PATH "/00000000.csv";

static SemaphoreHandle_t s_gps_queue_mtx;
static QueueHandle_t s_gps_queue_active;
static QueueHandle_t s_gps_queue_snapshot;

FILE *open_file(char *path)
{
    ESP_LOGI(TAG, "Opening file %s", path);
    FILE *f = fopen(path, FILE_MODE);
    if (f == NULL)
    {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return NULL;
    }
    return f;
}

void write_gps_queue_to_file()
{
    FILE *f = open_file(s_gps_csv_log_path);

    if (f == NULL)
    {
        return;
    }

    QueueHandle_t temp = s_gps_queue_active;
    s_gps_queue_active = s_gps_queue_snapshot;
    s_gps_queue_snapshot = temp;

    char *gps_row_str;
    while (xQueueReceive(s_gps_queue_snapshot, &gps_row_str, 0) != errQUEUE_EMPTY)
    {
        fprintf(f, "%s\n", gps_row_str);
        free(gps_row_str);
    }

    fclose(f);
    ESP_LOGI(TAG, "File written");
}

void task_sd_card(void *pvParameters)
{
    s_gps_queue_mtx = xSemaphoreCreateMutex();
    s_gps_queue_active = xQueueCreate(QUEUE_GPS_LENGTH, sizeof(char *));
    s_gps_queue_snapshot = xQueueCreate(QUEUE_GPS_LENGTH, sizeof(char *));

    ESP_LOGI(TAG, "Initializing SDMMC Host");
    sdmmc_slot_config_t slot = SDMMC_SLOT_CONFIG_DEFAULT();
    slot.width = 4;
    slot.clk = GPIO_NUM_43;
    slot.cmd = GPIO_NUM_44;
    slot.d0 = GPIO_NUM_39;
    slot.d1 = GPIO_NUM_40;
    slot.d2 = GPIO_NUM_41;
    slot.d3 = GPIO_NUM_42;
    sd_pwr_ctrl_ldo_config_t ldo_config = {
        .ldo_chan_id = 4,
    };
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.slot = SDMMC_HOST_SLOT_1;
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
    sd_pwr_ctrl_handle_t pwr_handle = NULL;
    ESP_ERROR_CHECK(sd_pwr_ctrl_new_on_chip_ldo(&ldo_config, &pwr_handle));
    host.pwr_ctrl_handle = pwr_handle;
    ESP_LOGI(TAG, "Initialized SDMMC Host Successfully");

    ESP_LOGI(TAG, "Mounting VFS FAT SDMMC");
    sdmmc_card_t out_card = {0};
    esp_vfs_fat_mount_config_t vfs_fat_mount_config = {
        .max_files = 1,
    };
    sdmmc_card_t *out_card_ptr = &out_card;
    ESP_ERROR_CHECK(esp_vfs_fat_sdmmc_mount(SD_MOUNT_PATH,
                                            &host,
                                            &slot,
                                            &vfs_fat_mount_config,
                                            &out_card_ptr));
    ESP_LOGI(TAG, "Mounted VFS FAT SDMMC Successfully");

    sdmmc_card_print_info(stdout, out_card_ptr);

    const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;
    while (true)
    {
        if (xSemaphoreTake(s_gps_queue_mtx, QUEUE_TIMEOUT) == pdTRUE)
        {
            bool ready = uxQueueMessagesWaiting(s_gps_queue_active) >= QUEUE_MIN_ITEMS_TO_WRITE;

            if (ready)
            {
                write_gps_queue_to_file();
            }

            if (xSemaphoreGive(s_gps_queue_mtx) == pdFALSE)
            {
                ESP_LOGE(TAG, "Could not give gps_queue_semphr in task_sd_card");
            }

            if (!ready)
            {
                vTaskDelay(xDelay);
            }
        }
        else
        {
            ESP_LOGE(TAG, "Could not take gps_queue_semphr in task_sd_card");
        }
    }

    sdmmc_host_deinit();
    vTaskDelete(NULL);
}

void enqueue_gps(char *data)
{
    if (xSemaphoreTake(s_gps_queue_mtx, QUEUE_TIMEOUT) == pdTRUE)
    {
        if (xQueueSendToBack(s_gps_queue_active, &data, 0) != pdPASS)
        {
            free(data);
        }

        if (xSemaphoreGive(s_gps_queue_mtx) == pdFALSE)
        {
            ESP_LOGE(TAG, "Could not give gps_queue_semphr after enqueuing gps");
        }
    }
    else
    {
        ESP_LOGE(TAG, "Could not take gps_queue_semphr to enqeue gps");
    }
}
