idf_component_register(SRCS "OTA.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_http_client app_update nvs_flash esp_netif)

