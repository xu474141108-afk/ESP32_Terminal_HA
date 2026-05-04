#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void storage_manager_init();
void save_devices_to_nvs(void);
void load_devices_from_nvs(void);

#ifdef __cplusplus
}
#endif