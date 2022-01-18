/**
 * Copyright (C) 2022 ael-mess
 *
 * @file    app_mqtt.c
 * @brief   MQTT driver.
 * @author  ael-mess
 *
 * @addtogroup NET
 * @{
 */

#include "string.h"

#include "esp_system.h"
#include "esp_event.h"
#include "mqtt_client.h"

#include "app_log.h"
#if CONFIG_ENABLE_LOGGING
static const char* TAG = "app-mqtt";
#endif

/*===========================================================================*/
/* Local definitions.                                                        */
/*===========================================================================*/
#define BROKER_HOST  CONFIG_BROKER_HOST
#define BROKER_PORT  CONFIG_BROKER_PORT
#define BROKER_TOPIC CONFIG_BROKER_TOPIC
#define DEVICE_ID    CONFIG_DEVICE_ID
#define DEVICE_KEY   CONFIG_DEVICE_KEY

/*===========================================================================*/
/* Local variables.                                                          */
/*===========================================================================*/
static esp_mqtt_client_handle_t m_client;

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/
static void mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
    switch (event->event_id) {
    case MQTT_EVENT_CONNECTED:
        RTN_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        break;
    case MQTT_EVENT_DISCONNECTED:
        RTN_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;
    case MQTT_EVENT_SUBSCRIBED:
        RTN_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        RTN_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        RTN_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        RTN_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        RTN_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        RTN_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

static void mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data) {
    mqtt_event_handler_cb(event_data);
}

/**
 * @brief   Publish person counter.
 *
 * @param[in] count person counter
 * @return          retrun msg
 *
 */
void app_mqtt_publish(uint8_t count) {
    char data[128] = {'\0'};
    sprintf(data, "{ \"type\": \"count\", \"value\": %d }", count);

    char topic[128] = {'\0'};
    sprintf(topic, BROKER_TOPIC, DEVICE_ID);

    int msg_id = esp_mqtt_client_publish(m_client, topic, data, 0, 1, 0);
    RTN_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
}

/**
 * @brief   Initialize MQTT.
 *
 * @return  retrun msg
 *
 */
esp_err_t app_mqtt_start(uint8_t mac[6]) {
    RTN_LOGI(TAG, "Initializing mqtt");

    char name[128] = {'\0'};
    sprintf(name, "%x%x%x%x%x%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    esp_mqtt_client_config_t mqtt_cfg = {
        .host      = BROKER_HOST,
        .port      = BROKER_PORT,
        .client_id = DEVICE_ID,
        .username  = name,
        .password  = DEVICE_KEY,
    };

    m_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(m_client, ESP_EVENT_ANY_ID, mqtt_event_handler, m_client);
    return esp_mqtt_client_start(m_client);
}

/** @} */
