/**
 * Copyright (C) 2022 ael-mess
 *
 * @file    app_nvs.h
 * @brief   NVS driver.
 * @author  ael-mess
 *
 * @addtogroup IN
 * @{
 */

#ifndef _APP_NVS_H_
#define _APP_NVS_H_

/*===========================================================================*/
/* External definitions.                                                     */
/*===========================================================================*/
#define ESP_WIFI_SSID_SIZE 32
#define ESP_WIFI_PASS_SIZE 64

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t app_nvs_init(char* wifi_ssid, char* wifi_pass, char* ap_ssid, char* ap_pass);
esp_err_t app_nvs_set_wifi(char* ssid, char* pass);
esp_err_t app_nvs_set_ap(char* ssid, char* pass);
esp_err_t app_nvs_set_counter(uint16_t number);
uint16_t  app_nvs_get_counter(void);

#ifdef __cplusplus
}
#endif

#endif /* _APP_NVS_H_ */

/** @} */
