#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_log.h"
#include "esp_err.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"

#include "sd_pwr_ctrl_by_on_chip_ldo.h"

#include "sd_card_iface.h"
#include "linked_list/include/linked_list.h"

#define TAG "sd_card_iface"
#define SD_MOUNT_PATH "/sdcard"
#define FILE_PATH_GPS SD_MOUNT_PATH "/gps.csv"
#define MUTEX_TIMEOUT 5000 / portTICK_PERIOD_MS
#define FILE_MODE "a"

typedef struct linked_list LineQueue;

static LineQueue *gps_queue;
static LineQueue *gps_snapshot_queue;
static SemaphoreHandle_t gps_queue_mtx;

typedef enum queue_type : int
{
    GPS = 1
} QueueType;

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
    {
        if (xSemaphoreTake(gps_queue_mtx, MUTEX_TIMEOUT) == pdFALSE)
        {
            ESP_LOGE(TAG, "Could not aquire mutex to write gps to file");
            return;
        }
        LineQueue *empty_queue = gps_snapshot_queue;
        gps_snapshot_queue = gps_queue;
        gps_queue = empty_queue;
        if (xSemaphoreGive(gps_queue_mtx) == pdFALSE)
        {
            ESP_LOGE(TAG, "Could not release mutex to write gps to file");
            return;
        }
    }
    FILE *f = open_file(FILE_PATH_GPS);

    if (f == NULL)
    {
        return;
    }

    Node *it = gps_snapshot_queue->front;
    while (it != NULL)
    {
        fprintf(f, "%s\n", (char*)it->data);
        it = it->next;
    }

    fclose(f);
    ESP_LOGI(TAG, "File written");

    clear_linked_list(gps_snapshot_queue);
}

void task_sd_card(void *pvParameters)
{
    gps_queue_mtx = xSemaphoreCreateMutex();
    gps_queue = make_linked_list(GPS);
    gps_snapshot_queue = make_linked_list(GPS);

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
        if (gps_queue->size > 5)
        {
            write_gps_queue_to_file();
        }

        vTaskDelay(xDelay);
    }

    destroy_linked_list(&gps_queue);
    destroy_linked_list(&gps_snapshot_queue);

    sdmmc_host_deinit();

    vSemaphoreDelete(gps_queue_mtx);
    vTaskDelete(NULL);
}

void enqueue_data(char *data, size_t data_size, QueueType queue_type)
{
    Node *node = make_node();

    node->data = (char *)calloc(data_size + 1, sizeof(char));
    strncpy(node->data, data, data_size);

    switch (queue_type)
    {
    case GPS:
        push_back_take(gps_queue, &node, queue_type);
        break;
    }
}

void enqueue_gps(char *data, size_t data_size)
{
    if (xSemaphoreTake(gps_queue_mtx, MUTEX_TIMEOUT) == pdFALSE)
    {
        ESP_LOGE(TAG, "Could not aquire mutex to enqueue gps");
        return;
    }

    enqueue_data(data, data_size, GPS);

    if (xSemaphoreGive(gps_queue_mtx) == pdFALSE)
    {
        ESP_LOGE(TAG, "Could not release mutex after enqueued gps");
        return;
    }
}
