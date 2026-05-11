idf_component_register(SRCS "OTA.c"
                       INCLUDE_DIRS "."
                       REQUIRES esp_http_client app_update
                       PRIV_REQUIRES nvs_flash esp_netif common_types
                    #    EMBED_TXTFILES "ca_cert.pem"
                       )

