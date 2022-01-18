/**
 * Copyright (C) 2022 ael-mess
 *
 * @file    app_wifi.c
 * @brief   WiFi driver.
 * @author  ael-mess
 *
 * @addtogroup HW
 * @{
 */

#include "stdbool.h"
#include "string.h"

#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_wifi.h"

#include "app_nvs.h"

#include "app_log.h"
#if CONFIG_ENABLE_LOGGING
static const char* TAG = "app-wifi";
#endif

/*===========================================================================*/
/* Local definitions.                                                        */
/*===========================================================================*/
#define ESP_WIFI_MAXIMUM_RETRY CONFIG_ESP_WIFI_MAXIMUM_RETRY // not used for now
#define ESP_WIFI_MAX_STA_CONN  CONFIG_ESP_WIFI_MAX_STA_CONN
#define ESP_WIFI_AP_IP_ADDR    CONFIG_ESP_WIFI_AP_IP
#define ESP_WIFI_AP_CHANNEL    CONFIG_ESP_WIFI_AP_CHANNEL

/*===========================================================================*/
/* Local variables.                                                          */
/*===========================================================================*/
static bool m_connected = false;

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/
static void on_got_ip(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
#if CONFIG_ENABLE_LOGGING
    ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
    RTN_LOGI(TAG, "IPv4 address: " IPSTR, IP2STR(&event->ip_info.ip));
#endif
    m_connected = true;
}

#if CONFIG_ESP_WIFI_CONNECT_IPV6
static void on_got_ipv6(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
#if CONFIG_ENABLE_LOGGING
    ip_event_got_ip6_t* event = (ip_event_got_ip6_t*)event_data;
    RTN_LOGI(TAG, "IPv6 address: " IPV6STR, IPV62STR(event->ip6_info.ip));
#endif
    m_connected = true;
}

static void on_wifi_connectv6(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    tcpip_adapter_create_ip6_linklocal(TCPIP_ADAPTER_IF_STA);
}
#endif

static void on_wifi_start(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    ESP_ERROR_CHECK(esp_wifi_connect());
    RTN_LOGI(TAG, "Wi-Fi connected");
}

static void on_wifi_disconnect(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    RTN_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect...");
    m_connected = false;
    ESP_ERROR_CHECK(esp_wifi_connect());
}

static void wifi_init_softap(const char* wifi_ssid, const char* wifi_pass) {
    esp_netif_create_default_wifi_ap();

    // TODO: check where 8 comes from
    char* password = "";
    if ((strlen(wifi_pass) > 8) && (strlen(wifi_pass) < ESP_WIFI_PASS_SIZE)) {
        password = (char*)wifi_pass;
    }

    if (strcmp(ESP_WIFI_AP_IP_ADDR, "192.168.4.1")) {
        int a, b, c, d;
        sscanf(ESP_WIFI_AP_IP_ADDR, "%d.%d.%d.%d", &a, &b, &c, &d);
        tcpip_adapter_ip_info_t ip_info;
        IP4_ADDR(&ip_info.ip, a, b, c, d);
        IP4_ADDR(&ip_info.gw, a, b, c, d);
        IP4_ADDR(&ip_info.netmask, 255, 255, 255, 0);
        ESP_ERROR_CHECK(tcpip_adapter_dhcps_stop(WIFI_IF_AP));
        ESP_ERROR_CHECK(tcpip_adapter_set_ip_info(WIFI_IF_AP, &ip_info));
        ESP_ERROR_CHECK(tcpip_adapter_dhcps_start(WIFI_IF_AP));
    }

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    snprintf((char*)wifi_config.ap.ssid, ESP_WIFI_SSID_SIZE, "%s", wifi_ssid);
    snprintf((char*)wifi_config.ap.password, ESP_WIFI_PASS_SIZE, "%s", password);

    wifi_config.ap.ssid_len       = strlen((char*)wifi_config.ap.ssid);
    wifi_config.ap.max_connection = ESP_WIFI_MAX_STA_CONN;
    wifi_config.ap.authmode       = WIFI_AUTH_WPA_WPA2_PSK;

    if (strlen(password) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    if (strlen(ESP_WIFI_AP_CHANNEL)) {
        int channel;
        sscanf(ESP_WIFI_AP_CHANNEL, "%d", &channel);
        wifi_config.ap.channel = channel;
    }

    RTN_LOGI(TAG, "SoftAP configured (SSID:%s password:%s)", wifi_ssid, password);
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
}

static esp_err_t wifi_init_sta(const char* wifi_ssid, const char* wifi_pass) {
    esp_netif_create_default_wifi_sta();

    char* password = "";
    if ((strlen(wifi_pass) > 8) && (strlen(wifi_pass) < ESP_WIFI_PASS_SIZE)) {
        password = (char*)wifi_pass;
    }

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    snprintf((char*)wifi_config.sta.ssid, ESP_WIFI_SSID_SIZE, "%s", wifi_ssid);
    snprintf((char*)wifi_config.sta.password, ESP_WIFI_PASS_SIZE, "%s", password);

    RTN_LOGI(TAG, "STA configured (SSID:%s password:%s)", wifi_ssid, password);
    return esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
}

/**
 * @brief   Initialize WiFi driver.
 *
 * @param[in] wifi_ssid STA WiFi SSID pointer (max size ESP_WIFI_SSID_SIZE)
 * @param[in] wifi_pass STA WiFi password pointer (max size ESP_WIFI_PASS_SIZE)
 * @param[in] ap_ssid   AP WiFi SSID pointer (max size ESP_WIFI_SSID_SIZE)
 * @param[in] ap_pass   AP WiFi password pointer (max size ESP_WIFI_PASS_SIZE)
 * @return              retrun msg
 *
 */
esp_err_t app_wifi_open(char* wifi_ssid, char* wifi_pass, char* ap_ssid, char* ap_pass) {
    RTN_LOGI(TAG, "Initializing WiFi ..");

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    RTN_LOGI(TAG, "Using (STASSID:%s password:%s) - (APSSID:%s password:%s)", wifi_ssid, wifi_pass, ap_ssid, ap_pass);

    wifi_mode_t mode = WIFI_MODE_NULL;
    if (strlen(ap_ssid) && strlen(wifi_ssid)) {
        mode = WIFI_MODE_APSTA;
    } else if (strlen(ap_ssid)) {
        mode = WIFI_MODE_AP;
    } else if (strlen(wifi_ssid)) {
        mode = WIFI_MODE_STA;
    }

    if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA) {
        RTN_LOGE(TAG, "STA WiFi mode cannot run without configuration");
        return ESP_FAIL;
    }

    if (mode == WIFI_MODE_NULL) {
        RTN_LOGW(TAG, "Neither AP or STA have been configured. WiFi will be off");
        return ESP_FAIL;
    }

#if CONFIG_ENABLE_LOGGING
    if (mode != WIFI_MODE_APSTA && mode != WIFI_MODE_AP) {
        RTN_LOGW(TAG, "AP WiFi mode not configured: This version cannot be "
                      "user in production");
    }
#endif

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(mode));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_START, &on_wifi_start, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip, NULL));
#if CONFIG_ESP_WIFI_CONNECT_IPV6
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &on_wifi_connectv6, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_GOT_IP6, &on_got_ipv6, NULL));
#endif

    if (mode & WIFI_MODE_AP) {
        wifi_init_softap(ap_ssid, ap_pass);
    }

    if (mode & WIFI_MODE_STA) {
        wifi_init_sta(wifi_ssid, wifi_pass);
    }

    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    return ESP_OK;
}

/**
 * @brief   Connexion state getter.
 *
 * @return  retrun connected
 *
 */
bool app_wifi_isconnected(void) { return m_connected; }

/**
 * @brief   MAC address getter.
 *
 * @param[out] mac  mac address
 * @return          retrun msg
 *
 */
esp_err_t app_wifi_getmac(uint8_t mac[6]) { return esp_wifi_get_mac(WIFI_IF_STA, mac); }

/**
 * @brief   Close WiFi driver.
 *
 */
void app_wifi_close(void) {
    RTN_LOGI(TAG, "Closing WiFi ..");

    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_START, &on_wifi_start));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_got_ip));
#if CONFIG_EXAMPLE_CONNECT_IPV6
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &on_wifi_connectv6));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_GOT_IP6, &on_got_ipv6));
#endif
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());

    RTN_LOGI(TAG, "Wi-Fi disconnected");
    m_connected = false;
}

/** @} */
