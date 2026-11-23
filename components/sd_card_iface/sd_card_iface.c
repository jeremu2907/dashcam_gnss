#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"

#include "sd_pwr_ctrl_by_on_chip_ldo.h"

#include "sd_card_iface.h"

#define TAG "sd_card_iface"
#define SD_MOUNT_PATH "/sdcard"

void task_sd_card(void* pvParameters)
{
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

    sdmmc_host_deinit();
    vTaskDelete(NULL);
}
