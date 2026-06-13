idf_component_register(SRCS "OTA.c" "OTA_MQTT.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_http_client app_update
                       PRIV_REQUIRES nvs_flash esp_netif mqtt json
                    #    EMBED_TXTFILES "ca_cert.pem"
                       )

