/**
 * Copyright (C) 2022 ael-mess
 *
 * @file    app_ota.h
 * @brief   OTA update driver.
 * @author  ael-mess
 *
 * @addtogroup IN
 * @{
 */

#ifndef _APP_OTA_H_
#define _APP_OTA_H_

#ifdef __cplusplus
extern "C" {
#endif

void      app_ota_check_boot(void);
void      app_ota_get_desc(char* version, char* name, char* time, char* date, char* idf_version);
esp_err_t app_ota_init(const char* ota_desc, const uint16_t length);
esp_err_t app_ota_write(const char* ota_data, const uint16_t length);
esp_err_t app_ota_update(void);

#ifdef __cplusplus
}
#endif

#endif /* _APP_OTA_H_ */

/** @} */
