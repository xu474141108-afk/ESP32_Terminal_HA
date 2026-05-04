idf_component_register(SRCS "ha_http_control.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES "esp_websocket_client" "json" "lvgl" "common_types" "esp_http_client")

