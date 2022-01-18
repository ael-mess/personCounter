/**
 * Copyright (C) 2022 ael-mess
 *
 * @file    app_mqtt.h
 * @brief   MQTT driver.
 * @author  ael-mess
 *
 * @addtogroup NET
 * @{
 */

#ifndef _APP_MQTT_H_
#define _APP_MQTT_H_

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t app_mqtt_start(uint8_t mac[6]);
void      app_mqtt_publish(uint8_t count);

#ifdef __cplusplus
}
#endif

#endif /* _APP_MQTT_H_ */

/** @} */
