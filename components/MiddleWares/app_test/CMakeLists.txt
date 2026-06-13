idf_component_register(SRCS "app_test.c"
                       INCLUDE_DIRS "."
                       REQUIRES driver
                       PRIV_REQUIRES lvgl ha_http_req mdns)

