set (INCLUDES ".""./generated" "./custom" "./generated/guider_customer_fonts" "./generated/images")
file(GLOB_RECURSE SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/*/*.c"
)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS ${INCLUDES}
                    REQUIRES lvgl common_types ha_http_control
                    PRIV_REQUIRES app_wifi esp_lcd_touch OTA
                    )