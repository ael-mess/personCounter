/**
 * Copyright (C) 2022 ael-mess
 *
 * @file    app_log.h
 * @brief   Application layer for ESP_LOG.
 * @author  ael-mess
 *
 * @addtogroup IN
 * @{
 */

#ifndef _APP_LOG_H_
#define _APP_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_ENABLE_LOGGING
#include "esp_log.h"

#define RTN_LOGI(...) ESP_LOGI(__VA_ARGS__)
#define RTN_LOGW(...) ESP_LOGW(__VA_ARGS__)
#define RTN_LOGE(...) ESP_LOGE(__VA_ARGS__)
#else
#define RTN_LOGI(...)                                                                                                  \
    {}
#define RTN_LOGW(...)                                                                                                  \
    {}
#define RTN_LOGE(...)                                                                                                  \
    {}
#endif

#ifdef __cplusplus
}
#endif

#endif /* _APP_LOG_H_ */

/** @} */
