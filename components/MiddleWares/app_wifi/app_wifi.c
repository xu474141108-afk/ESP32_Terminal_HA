//WiFi APSTA 基础。实现 AP 模式下开启 Web Server，实现重启后自动连接 STA。
#include "sdkconfig.h"
#include "sys/param.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_http_server.h"
//其他文件
#include "app_wifi.h"
#include "storage_manager.h"
#include "ha_ws_client.h"



static const char *TAG = "app_wifi";
wifi_sm_t g_wifi_sm;
static int s_retry_num = 0;
httpd_handle_t server = NULL;


wifi_config_t ap_cfg = {
                .ap = { 
                    .ssid = "ESP32_AP", 
                    .max_connection = 5, 
                    .authmode = WIFI_AUTH_WPA2_PSK, 
                    .password = "12345678" 
                }
            };


static esp_err_t config_get_handler(httpd_req_t *req) {
    const char* resp_str = 
               "<!DOCTYPE html><html><head>"
        "<meta charset=\"UTF-8\">"
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
        "<style>"
        "  body { display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; background-color: #f0f2f5; font-family: Arial, sans-serif; }"
        "  .card { background: white; padding: 40px 30px; border-radius: 12px; box-shadow: 0 4px 12px rgba(0,0,0,0.15); width: 80%; max-width: 380px; text-align: center; }"
        "  input[type=\"text\"], input[type=\"password\"] { width: 100%; padding: 15px; margin: 10px 0 20px 0; border: 1px solid #ccc; border-radius: 8px; box-sizing: border-box; font-size: 16px; }"
        "  input[type=\"submit\"] { width: 100%; background-color: #4CAF50; color: white; padding: 15px; border: none; border-radius: 8px; cursor: pointer; font-size: 18px; font-weight: bold; }"
        "  input[type=\"submit\"]:active { background-color: #45a049; }"
        "</style>"
        "</head><body>"
        "<div class=\"card\">"
        "  <h2 style=\"color: #333; margin-top: 0;\">Ajuste de red del dispositivo</h2>"
        "  <form action=\"/config\" method=\"post\">"
        "    <div style=\"text-align: left; color: #666; font-size: 14px;\">Nombre WiFi (SSID):</div>"
        "    <input type=\"text\" name=\"ssid\" placeholder=\"Introduce el nombre de la red WiFi\" required>"
        "    <div style=\"text-align: left; color: #666; font-size: 14px;\">Contraseña WiFi:</div>"
        "    <input type=\"password\" name=\"pass\" placeholder=\"Escribe la contraseña\">"

        "    <div style=\"text-align: left; color: #666; font-size: 14px;\">IP Home Assistant:</div>"
        "    <input type=\"text\" name=\"ha_ip\" placeholder=\"Dejar vacío para no modificar\" value=\"\">"
        
        "    <div style=\"text-align: left; color: #666; font-size: 14px;\">Token Home Assistant:</div>"
        "    <input type=\"text\" name=\"ha_token\" placeholder=\"Dejar vacío para no modificar\" value=\"\">"

        "    <input type=\"submit\" value=\"CONECTAR\">"
        "  </form>"
        "</div>"
        "</body></html>";

    // Envía la página HTML completa como respuesta al navegador
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
}
// 接收 POST 请求并尝试连接WiFi
static esp_err_t config_post_handler(httpd_req_t *req) {
    char buf[512];
    char ssid[32] = {0};
    char pass[64] = {0};
    char ha_ip[32] = {0};
    char ha_token[256] = {0};
    int ret = httpd_req_recv(req, buf, MIN(req->content_len, sizeof(buf) - 1));
    if (ret <= 0) { return ESP_FAIL; }
    buf[ret] = '\0'; 
     if (httpd_query_key_value(buf, "ha_ip", ha_ip, sizeof(ha_ip)) == ESP_OK &&
        httpd_query_key_value(buf, "ha_token", ha_token, sizeof(ha_token)) == ESP_OK) {
        if(strlen(ha_token) > 0) {
            strncpy(g_ha_ws_client_config.ha_token, ha_token, sizeof(g_ha_ws_client_config.ha_token) - 1);
            g_ha_ws_client_config.ha_token[sizeof(g_ha_ws_client_config.ha_token) - 1] = '\0'; // 防止溢出
            ha_date_item_save(g_ha_ws_client_config.ha_token);
        }
        if(strlen(ha_ip) > 0) {
            strncpy(g_ha_ws_client_config.ha_ip, ha_ip, sizeof(g_ha_ws_client_config.ha_ip) - 1);
            g_ha_ws_client_config.ha_ip[sizeof(g_ha_ws_client_config.ha_ip) - 1] = '\0'; // 防止溢出
            ha_date_item_save(g_ha_ws_client_config.ha_ip);
            esp_event_post(HA_ACTION_EVENTS, HA_TOKEN_IP_UPDATE, NULL, 0, portMAX_DELAY);
        }
    } else {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "HA IP or Token missing");
    }

    if (httpd_query_key_value(buf, "ssid", ssid, sizeof(ssid)) == ESP_OK &&
        httpd_query_key_value(buf, "pass", pass, sizeof(pass)) == ESP_OK) {
        
        ESP_LOGI(TAG, "解析成功: SSID=%s, Pass=%s", ssid, pass);

        // 3. 配置并连接
        wifi_config_t wifi_config_http = {0};
        strlcpy((char*)wifi_config_http.sta.ssid, ssid, sizeof(wifi_config_http.sta.ssid));
        strlcpy((char*)wifi_config_http.sta.password, pass, sizeof(wifi_config_http.sta.password));
        
        // 建议在设置前关闭一次连接，确保配置能生效
        esp_wifi_disconnect(); 
        esp_wifi_set_config(WIFI_IF_STA, &wifi_config_http);
        esp_wifi_connect();

        httpd_resp_send(req, "Connecting to WiFi...", -1);
    } else {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "SSID or Password missing");
    }

    return ESP_OK;
}

void webserver_begin() {
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if(mode == WIFI_MODE_APSTA) {
        ESP_LOGI(TAG, "WebServer already running in APSTA mode, stopping it");
        if (server) {
        httpd_stop(server);
        server = NULL;
    }
        return;
    }
    esp_wifi_set_mode(WIFI_MODE_APSTA);
    esp_wifi_set_config(WIFI_IF_AP, &ap_cfg);
        httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t get_uri = { .uri = "/", .method = HTTP_GET, .handler = config_get_handler };
        httpd_register_uri_handler(server, &get_uri);
        httpd_uri_t post_uri = { .uri = "/config", .method = HTTP_POST, .handler = config_post_handler };
        httpd_register_uri_handler(server, &post_uri);
    }
    g_wifi_sm.wifi_FSM_state = WIFI_STATE_PROVISIONING;
}

static void wifi_retry_backoff_task(void *param)
{
    uint32_t delay_ms;
    // 计算指数退避时间：delay = base * (2^retry_num)
    delay_ms = 1000 * (1 << s_retry_num);
    // 限制最大等待时长
    if (delay_ms > 1000)
    {
        delay_ms = 10000;
    }

    ESP_LOGI(TAG, "指数退避等待 %lu ms 后重试WiFi", delay_ms);
    vTaskDelay(pdMS_TO_TICKS(delay_ms));

    esp_err_t errt = esp_wifi_connect();
    ESP_LOGI(TAG, "esp_wifi_connect() 返回值: %d (%s)", errt, esp_err_to_name(errt));
    vTaskDelete(NULL);
}
//WiFi Callback
static void event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data) 
{
    esp_err_t errt;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        errt = esp_wifi_connect();
        ESP_LOGI(TAG, "esp_wifi_connect() value return: %d (%s)", errt, esp_err_to_name(errt));
        if (errt == ESP_ERR_WIFI_SSID) {
            ESP_LOGW(TAG, "No WiFi Config found，need to configure");
            g_wifi_sm.wifi_FSM_state = WIFI_STATE_NONVS_CONFIG;
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 20) {
            s_retry_num++;
            ESP_LOGI(TAG, "WiFi disconnected, retry, count:%d", s_retry_num);
            xTaskCreate(wifi_retry_backoff_task, "wifi_backoff", 4096, NULL, 3, NULL);
            g_wifi_sm.wifi_FSM_state = WIFI_STATE_STA_CONNECTING;
        } else {
            ESP_LOGW(TAG, "No WiFi disconnected, need to configure");
            g_wifi_sm.wifi_FSM_state = WIFI_STATE_DISCONNECTED;
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        
        // WiFi connected successfully, update state and send to UI
        g_wifi_sm.wifi_FSM_state = WIFI_STATE_CONNECTED;
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "Got IP address: " IPSTR, IP2STR(&event->ip_info.ip));
        snprintf(g_wifi_sm.wifi_ip, sizeof(g_wifi_sm.wifi_ip), IPSTR, IP2STR(&event->ip_info.ip));
        g_wifi_sm.wifi_ip[sizeof(g_wifi_sm.wifi_ip)-1] = '\0';
        wifi_config_t sta_conf;
        esp_wifi_get_config(WIFI_IF_STA, &sta_conf);
        strncpy(g_wifi_sm.wifi_ssid, (const char*)sta_conf.sta.ssid, sizeof(g_wifi_sm.wifi_ssid)-1);
        g_wifi_sm.wifi_ssid[sizeof(g_wifi_sm.wifi_ssid)-1] = '\0';
        s_retry_num = 0;

        
        // if WiFi is in APSTA mode, switch to STA only mode to free up AP resources
        wifi_mode_t mode;
        esp_wifi_get_mode(&mode);
        if (mode == WIFI_MODE_APSTA) {
            ESP_LOGI(TAG, "Freeing AP resources...");
            if (server) {
                httpd_stop(server);
                server = NULL;
            }
            esp_wifi_set_mode(WIFI_MODE_STA);
        }
    }
}

//WiFi init
void wifi_init(void)
{
    ESP_LOGI(TAG, "WIFI INIT BEGIN");
    //low-level initialization WiFi 
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));

    // Callback registration
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    // WiFi configuration
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); 
    wifi_config_t wifi_cfg = {0};
    esp_wifi_get_config(WIFI_IF_STA, &wifi_cfg);
    if (strlen((char*)wifi_cfg.sta.ssid) > 0) {
        ESP_LOGI(TAG, "NVS WiFi: %s", wifi_cfg.sta.ssid);
    }else {
        ESP_LOGW(TAG, "NVS NO WiFi.");
    }
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_cfg) );
    ESP_ERROR_CHECK(esp_wifi_start());  
    ESP_LOGI(TAG, "WIFI INIT FINISH");
}

