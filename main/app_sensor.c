/**
 * Copyright (C) 2022 ael-mess
 *
 * @file    app_sensor.c
 * @brief   Sensor driver.
 * @author  ael-mess
 *
 * @addtogroup HW
 * @{
 */

#include "esp_err.h"

#include "app_log.h"
#if CONFIG_ENABLE_LOGGING
static const char* TAG = "app_sensor";
#endif

#if !CONFIG_USE_DUMMY
#error "No sensor found for the moment, dummy data need to be enabled"
#endif

/**
 * @brief   Initialize sensor.
 *
 * @return   retrun msg
 *
 */
esp_err_t app_sensor_init(void) {
    RTN_LOGI(TAG, "Initializing sensor");

    return ESP_OK;
}

/**
 * @brief   Read person counter.
 *
 * @return  retrun person count
 *
 */
uint8_t app_sensor_get_count(void) {
    static uint8_t count = 0xff;
    count++;

    return count;
}

/** @} */
