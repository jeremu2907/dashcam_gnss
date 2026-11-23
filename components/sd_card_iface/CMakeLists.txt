idf_component_register(SRCS "sd_card_iface.c"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_driver_sdmmc
                                  sdmmc
                                  fatfs)
