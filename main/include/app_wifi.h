/**
 * Copyright (C) 2022 ael-mess
 *
 * @file    app_wifi.h
 * @brief   WiFi driver.
 * @author  ael-mess
 *
 * @addtogroup HW
 * @{
 */

#ifndef _APP_WIFI_H_
#define _APP_WIFI_H_

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t app_wifi_open(char* wifi_ssid, char* wifi_pass, char* ap_ssid, char* ap_pass);
void      app_wifi_close(void);

#ifdef __cplusplus
}
#endif

#endif /* _APP_WIFI_H_ */

/** @} */
