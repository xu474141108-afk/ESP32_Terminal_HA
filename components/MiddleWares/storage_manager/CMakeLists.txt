idf_component_register(SRCS "storage_manager.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES ha_http_control common_types nvs_flash)

