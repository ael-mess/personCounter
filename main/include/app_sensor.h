/**
 * Copyright (C) 2022 ael-mess
 *
 * @file    app_sensor.h
 * @brief   Sensor driver.
 * @author  ael-mess
 *
 * @addtogroup HW
 * @{
 */

#ifndef _APP_SENSOR_H_
#define _APP_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t app_sensor_init(void);
uint8_t app_sensor_count_get(void);

#ifdef __cplusplus
}
#endif

#endif /* _APP_SENSOR_H_ */

/** @} */
