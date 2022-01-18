/**
 * Copyright (C) 2022 ael-mess
 *
 * @file    app_main.c
 * @brief   Startup.
 * @author  ael-mess
 *
 * @addtogroup MAIN
 * @{
 */

#include "esp_err.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "app_nvs.h"
#include "app_wifi.h"
#include "app_mqtt.h"
#include "app_sensor.h"
#include "app_ota.h"

#define WIFI_SSID CONFIG_ESP_WIFI_SSID
#define WIFI_PASS CONFIG_ESP_WIFI_PASSWORD

void app_main(void) {
    app_ota_check_boot();

    // TODO: store WiFi config and counter later
    app_nvs_init(NULL, NULL, NULL, NULL);

    // TODO: use non blocking loop
    ESP_ERROR_CHECK(app_wifi_open(WIFI_SSID, WIFI_PASS, "", ""));
    while (!app_wifi_isconnected()) {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    uint8_t mac[6] = {0};
    app_wifi_getmac(mac);
    ESP_ERROR_CHECK(app_mqtt_start(mac));

    while (app_wifi_isconnected()) {
        app_mqtt_publish(app_sensor_get_count());
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/** @} */
