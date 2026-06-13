#pragma once


#ifdef __cplusplus
extern "C" {
#endif

void test_cpu_usage_task(void *arg);
esp_err_t mdns_find_homeassistant(char *out_ip, size_t max_len);

#ifdef __cplusplus
}
#endif