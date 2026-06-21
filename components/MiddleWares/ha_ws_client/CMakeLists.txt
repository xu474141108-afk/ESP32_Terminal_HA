idf_component_register(SRCS "ha_ws_client.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_event
                       PRIV_REQUIRES esp_websocket_client json lvgl lvgl_ui app_test ha_http_req)

