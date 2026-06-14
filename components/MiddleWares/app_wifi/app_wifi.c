//WiFi APSTA 基础。实现 AP 模式下开启 Web Server，实现重启后自动连接 STA。
#include "sdkconfig.h"
#include "sys/param.h"
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"

#include "esp_wifi.h"
#include "esp_http_server.h"
//其他文件
#include "app_wifi.h"

#define WIFI_CONNECTED_BIT BIT0
#define ESP_WIFI_SSID      "ESP32_AP"
#define ESP_WIFI_PASS      "12345678"
#define ESP_WIFI_MAXIMUM_CONNECTIONS 5
#define MAX_RETRY       10

static const char *TAG = "app_wifi";
wifi_sm_t g_wifi_sm;
static int s_retry_num = 0;
httpd_handle_t server = NULL;
EventGroupHandle_t s_wifi_event_group = NULL;
QueueHandle_t g_ui_status_queue = NULL;
wifi_config_t ap_cfg = {
                .ap = { 
                    .ssid = ESP_WIFI_SSID, 
                    .max_connection = ESP_WIFI_MAXIMUM_CONNECTIONS, 
                    .authmode = WIFI_AUTH_WPA2_PSK, 
                    .password = ESP_WIFI_PASS 
                }
            };

static esp_err_t config_get_handler(httpd_req_t *req) {
    const char* resp_str = 
        "<!DOCTYPE html><html><head>"
        "<meta charset=\"UTF-8\">"
        ""
        "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">"
        "<style>"
        "  /* El cuerpo usa flex para centrar todo el contenido en pantalla */"
        "  body { display: flex; justify-content: center; align-items: center; height: 100vh; margin: 0; background-color: #f0f2f5; font-family: Arial, sans-serif; }"
        "  /* Tarjeta con fondo blanco para el formulario */"
        "  .card { background: white; padding: 40px 30px; border-radius: 12px; box-shadow: 0 4px 12px rgba(0,0,0,0.15); width: 80%; max-width: 350px; text-align: center; }"
        "  /* Campos de texto ampliados con margen interno */"
        "  input[type=\"text\"], input[type=\"password\"] { width: 100%; padding: 15px; margin: 10px 0 20px 0; border: 1px solid #ccc; border-radius: 8px; box-sizing: border-box; font-size: 16px; }"
        "  /* Botón de envío grande con fondo verde */"
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
    char buf[100];
    char ssid[32] = {0};
    char pass[64] = {0};
    int ret = httpd_req_recv(req, buf, MIN(req->content_len, sizeof(buf) - 1));
    if (ret <= 0) { return ESP_FAIL; }
    buf[ret] = '\0'; 
    ESP_LOGI(TAG, "收到原始数据: %s", buf);
    
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

static void start_webserver() {
    if (server != NULL) return;
    g_wifi_sm.wifi_FSM_state = WIFI_STATE_PROVISIONING;
    xQueueSend(g_ui_status_queue, &g_wifi_sm.wifi_FSM_state, 0);
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t get_uri = { .uri = "/", .method = HTTP_GET, .handler = config_get_handler };
        httpd_register_uri_handler(server, &get_uri);
        httpd_uri_t post_uri = { .uri = "/config", .method = HTTP_POST, .handler = config_post_handler };
        httpd_register_uri_handler(server, &post_uri);
    }
}

static void stop_webserver() {
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
}

void webserver_begin() {
    wifi_mode_t mode;
    esp_wifi_get_mode(&mode);
    if(mode == WIFI_MODE_APSTA) {
        ESP_LOGI(TAG, "WebServer 已经启动，无需重复启动");
        return;
    }
    esp_wifi_set_mode(WIFI_MODE_APSTA);
    esp_wifi_set_config(WIFI_IF_AP, &ap_cfg);
    start_webserver();
    xEventGroupSetBits(s_wifi_event_group, BIT1);
}

// --- Wi-Fi 事件处理 ---
static void event_handler(void* arg, esp_event_base_t event_base,int32_t event_id, void* event_data) 
{
    esp_err_t errt;
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        errt = esp_wifi_connect();
        ESP_LOGI(TAG, "esp_wifi_connect() 返回值: %d (%s)", errt, esp_err_to_name(errt));
        if (errt == ESP_ERR_WIFI_SSID) {
            ESP_LOGW(TAG, "检测到未配置WiFi，请在设置中进行配网...");
            g_wifi_sm.wifi_FSM_state = WIFI_STATE_NONVS_CONFIG;
            xQueueSend(g_ui_status_queue, &g_wifi_sm.wifi_FSM_state, 0);
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MAX_RETRY) {
            BaseType_t res = xTaskCreate(wifi_retry_backoff_task, "wifi_backoff", 2048, NULL, 3, NULL);
            errt = esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "重试连接路由器...");
            g_wifi_sm.wifi_FSM_state = WIFI_STATE_STA_CONNECTING;
            xQueueSend(g_ui_status_queue, &g_wifi_sm.wifi_FSM_state, 0);
        } else {
            ESP_LOGW(TAG, "无法连接，请在设置中进行配网...");
            g_wifi_sm.wifi_FSM_state = WIFI_STATE_DISCONNECTED;
            xQueueSend(g_ui_status_queue, &g_wifi_sm.wifi_FSM_state, 0);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "成功获取 IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        g_wifi_sm.wifi_FSM_state = WIFI_STATE_CONNECTED;
        xQueueSend(g_ui_status_queue, &g_wifi_sm.wifi_FSM_state, 0);
        
        // 如果是从 APSTA 模式连接成功的，则切回 STA 并关闭 WebServer
        wifi_mode_t mode;
        esp_wifi_get_mode(&mode);
        if (mode == WIFI_MODE_APSTA) {
            ESP_LOGI(TAG, "配网成功，回收 AP 资源...");
            stop_webserver();
            esp_wifi_set_mode(WIFI_MODE_STA);
        }
        xEventGroupSetBits(s_wifi_event_group, BIT0);
    }
}


void wifi_init(void)
{
    ESP_LOGI(TAG, "WIFI INIT BEGIN");
    g_ui_status_queue = xQueueCreate(5, sizeof(uint8_t));
    s_wifi_event_group = xEventGroupCreate();
    //low-level initialization WiFi 
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    esp_netif_create_default_wifi_ap();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Callback registration
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    // WiFi configuration
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    wifi_config_t wifi_config = {0};
    esp_wifi_get_config(WIFI_IF_STA, &wifi_config);
    if (strlen((char*)wifi_config.sta.ssid) > 0) {
        ESP_LOGI(TAG, "NVS WiFi: %s", wifi_config.sta.ssid);
    }else {
        ESP_LOGW(TAG, "NVS NO WiFi.");
    }
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());  
    ESP_LOGI(TAG, "WIFI INIT FINISH");
}
