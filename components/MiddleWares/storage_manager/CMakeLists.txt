idf_component_register(SRCS "storage_manager.c"
                       INCLUDE_DIRS "."
                       REQUIRES ha_ws_client
                       PRIV_REQUIRES nvs_flash lvgl_ui)

