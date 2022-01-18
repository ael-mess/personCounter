/**
 * Copyright (C) 2022 ael-mess
 *
 * @file    app_nvs.c
 * @brief   NVS driver.
 * @author  ael-mess
 *
 * @addtogroup IN
 * @{
 */

#include "string.h"

#include "sdkconfig.h"
#include "esp_system.h"
#include "nvs.h"
#include "nvs_flash.h"

#include "app_nvs.h"

#include "app_log.h"
#if CONFIG_ENABLE_LOGGING
static const char* TAG = "app-nvs";
#endif

/*===========================================================================*/
/* Local definitions.                                                        */
/*===========================================================================*/
#define WIFI_STA_SSID_KEY "wifi_ssid"
#define WIFI_STA_PASS_KEY "wifi_pass"
#define WIFI_AP_SSID_KEY  "softap_ssid"
#define WIFI_AP_PASS_KEY  "softap_pass"
#define PERSON_COUNTER    "pers_count"

/*===========================================================================*/
/* Local variables.                                                          */
/*===========================================================================*/
static nvs_handle_t handle;

/**
 * @brief   Store WiFi configuration in NVS.
 *
 * @param[in] ssid  STA SSID pointer (max size ESP_WIFI_SSID_SIZE)
 * @param[in] pass  STA password pointer (max size ESP_WIFI_PASS_SIZE)
 * @return          retrun msg
 *
 */
esp_err_t app_nvs_set_wifi(char* ssid, char* pass) {
    esp_err_t ret = nvs_open("storage", NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        RTN_LOGE(TAG, "Cannot open Non-Volatile Storage");
        return ESP_FAIL;
    }

    esp_err_t ret1 = nvs_set_str(handle, WIFI_STA_SSID_KEY, ssid);
    esp_err_t ret2 = nvs_set_str(handle, WIFI_STA_PASS_KEY, pass);

    ret = nvs_commit(handle);
    nvs_close(handle);

    if ((ret == ESP_OK) && (ret1 == ESP_OK) && (ret2 == ESP_OK)) {
        RTN_LOGI(TAG, "NVS data set successfully");
        return ESP_OK;
    } else {
        RTN_LOGI(TAG, "Failed to set NVS data");
        return ESP_FAIL;
    }
}

/**
 * @brief   Store WiFi AP configuration in NVS.
 *
 * @param[in] ssid  AP SSID pointer (max size ESP_WIFI_SSID_SIZE)
 * @param[in] pass  AP password pointer (max size ESP_WIFI_PASS_SIZE)
 * @return          retrun msg
 *
 */
esp_err_t app_nvs_set_ap(char* ssid, char* pass) {
    esp_err_t ret = nvs_open("storage", NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        RTN_LOGE(TAG, "Cannot open Non-Volatile Storage");
        return ESP_FAIL;
    }

    esp_err_t ret1 = nvs_set_str(handle, WIFI_AP_SSID_KEY, ssid);
    esp_err_t ret2 = nvs_set_str(handle, WIFI_AP_PASS_KEY, pass);

    ret = nvs_commit(handle);
    nvs_close(handle);

    if ((ret == ESP_OK) && (ret1 == ESP_OK) && (ret2 == ESP_OK)) {
        RTN_LOGI(TAG, "NVS SoftAP data set successfully");
        return ESP_OK;
    } else {
        RTN_LOGI(TAG, "Failed to set NVS data");
        return ESP_FAIL;
    }
}

/**
 * @brief   Store person counter in NVS.
 *
 * @param[in] number    person counter
 * @return              retrun msg
 *
 */
esp_err_t app_nvs_set_counter(uint16_t number) {
    esp_err_t ret = nvs_open("storage", NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        RTN_LOGE(TAG, "Cannot open Non-Volatile Storage");
        return ESP_FAIL;
    }

    esp_err_t ret1 = nvs_set_u16(handle, PERSON_COUNTER, number);

    ret = nvs_commit(handle);
    nvs_close(handle);

    if ((ret == ESP_OK) && (ret1 == ESP_OK)) {
        RTN_LOGI(TAG, "NVS person counter data set to %d successfully", number);
        return ESP_OK;
    } else {
        RTN_LOGI(TAG, "Failed to set NVS person counter data");
        return ESP_FAIL;
    }
}

/**
 * @brief   Load prediction counter in NVS.
 *
 * @return  prediction counter
 *
 */
uint16_t app_nvs_get_counter(void) {
    if (nvs_open("storage", NVS_READWRITE, &handle) != ESP_OK) {
        RTN_LOGE(TAG, "Cannot open Non-Volatile Storage");
        return 0;
    }

    uint16_t count = 0;
    nvs_get_u16(handle, PERSON_COUNTER, &count);

    nvs_close(handle);
    return count;
}

/**
 * @brief   Initialize NVS.
 *
 * @param[in] wifi_ssid STA WiFi SSID pointer
 * @param[in] wifi_pass STA WiFi password pointer
 * @param[in] ap_ssid   AP WiFi SSID pointer
 * @param[in] ap_pass   AP WiFI password pointer
 * @return              retrun msg
 *
 */
esp_err_t app_nvs_init(char* wifi_ssid, char* wifi_pass, char* ap_ssid, char* ap_pass) {
    RTN_LOGI(TAG, "Initializing NVS ..");

    // TODO: check why memset not working
    /*memset(wifi_ssid, '\0', ESP_WIFI_SSID_SIZE + 1);
    memset(wifi_pass, '\0', ESP_WIFI_PASS_SIZE + 1);*/

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ret = nvs_open("storage", NVS_READWRITE, &handle);
    if (ret != ESP_OK) {
        RTN_LOGE(TAG, "Cannot open Non-Volatile Storage");
        return ESP_FAIL;
    }

    esp_err_t ret1 = ESP_ERR_NVS_NOT_FOUND;
    size_t    wifi_sta_ssid_lg;
    ret = nvs_get_str(handle, WIFI_STA_SSID_KEY, NULL, &wifi_sta_ssid_lg);
    if ((ret == ESP_OK) && (wifi_sta_ssid_lg <= (ESP_WIFI_SSID_SIZE + 1))) {
        ret1 = nvs_get_str(handle, WIFI_STA_SSID_KEY, wifi_ssid, &wifi_sta_ssid_lg);
    }

    esp_err_t ret2 = ESP_ERR_NVS_NOT_FOUND;
    size_t    wifi_sta_pass_lg;
    ret = nvs_get_str(handle, WIFI_STA_PASS_KEY, NULL, &wifi_sta_pass_lg);
    if ((ret == ESP_OK) && (wifi_sta_pass_lg <= (ESP_WIFI_PASS_SIZE + 1))) {
        ret2 = nvs_get_str(handle, WIFI_STA_PASS_KEY, wifi_pass, &wifi_sta_pass_lg);
    }

    size_t wifi_ap_ssid_lg;
    ret = nvs_get_str(handle, WIFI_AP_SSID_KEY, NULL, &wifi_ap_ssid_lg);
    if ((ret == ESP_OK) && (wifi_ap_ssid_lg <= (ESP_WIFI_SSID_SIZE + 1))) {
        nvs_get_str(handle, WIFI_AP_SSID_KEY, ap_ssid, &wifi_ap_ssid_lg);
    }

    size_t wifi_ap_pass_lg;
    ret = nvs_get_str(handle, WIFI_AP_PASS_KEY, NULL, &wifi_ap_pass_lg);
    if ((ret == ESP_OK) && (wifi_ap_pass_lg <= (ESP_WIFI_PASS_SIZE + 1))) {
        nvs_get_str(handle, WIFI_AP_PASS_KEY, ap_pass, &wifi_ap_pass_lg);
    }

    nvs_close(handle);

    if ((ret1 == ESP_OK) && (ret2 == ESP_OK)) {
        RTN_LOGI(TAG, "NVS data recovered successfully: %s-%s", wifi_ssid, wifi_pass);
        return ESP_OK;
    } else {
        RTN_LOGI(TAG, "NVS data not found");
        return ESP_FAIL;
    }
}

/** @} */
