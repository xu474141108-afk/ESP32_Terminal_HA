idf_component_register(SRCS "ha_ws_client.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES esp_websocket_client json lvgl lvgl_ui app_test)

