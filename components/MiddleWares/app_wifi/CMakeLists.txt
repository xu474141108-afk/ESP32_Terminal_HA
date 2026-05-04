idf_component_register(SRCS "app_wifi.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver
                       PRIV_REQUIRES esp_wifi esp_event esp_netif esp_http_server nvs_flash common_types)

