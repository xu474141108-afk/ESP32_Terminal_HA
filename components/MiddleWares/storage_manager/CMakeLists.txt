idf_component_register(SRCS "storage_manager.c"
                       INCLUDE_DIRS "."
                       REQUIRES ha_http_req
                       PRIV_REQUIRES nvs_flash lvgl_ui ha_ws_client)

