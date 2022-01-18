/**
 * Copyright (C) 2022 ael-mess
 *
 * @file    app_ota.c
 * @brief   OTA update driver.
 * @author  ael-mess
 *
 * @addtogroup IN
 * @{
 */

#include "string.h"

#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"

#include "errno.h"
#include "app_log.h"
#if CONFIG_ENABLE_LOGGING
static const char* TAG = "app_ota";
#endif

/*===========================================================================*/
/* Local definitions.                                                        */
/*===========================================================================*/
#define HASH_LEN             32 /* SHA-256 digest length */
#define OTA_MSG_DESC_LENGTH  (ESP_FAIL - 1)
#define OTA_MSG_NO_PARTITION (ESP_FAIL - 2)
#define OTA_MSG_INVALID_APP  (ESP_FAIL - 3)
#define OTA_MSG_SAME_VERSION (ESP_FAIL - 4)
#define OTA_MSG_INVALID_IMG  (ESP_FAIL - 5)

/*===========================================================================*/
/* Local variables.                                                          */
/*===========================================================================*/
static esp_ota_handle_t       update_handle      = 0;
static const esp_partition_t* update_partition   = NULL;
static int                    binary_file_length = 0;

/*===========================================================================*/
/* Local functions.                                                          */
/*===========================================================================*/
#if CONFIG_ENABLE_LOGGING
static void print_sha256(const uint8_t* image_hash, const char* label) {
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    RTN_LOGI(TAG, "%s: %s", label, hash_print);
}
#endif

/**
 * @brief   Get boot image information.
 *
 * @param[out] version      image version pointer
 * @param[out] name         image name pointer
 * @param[out] time         image time pointer
 * @param[out] date         image date pointer
 * @param[out] idf_version  IDF version pointer
 *
 */
void app_ota_get_desc(char* version, char* name, char* time, char* date, char* idf_version) {
    esp_app_desc_t app_desc;
    if (esp_ota_get_partition_description(esp_ota_get_running_partition(), &app_desc) == ESP_OK) {
        strcpy(version, app_desc.version);
        strcpy(name, app_desc.project_name);
        strcpy(time, app_desc.time);
        strcpy(date, app_desc.date);
        strcpy(idf_version, app_desc.idf_ver);
    }
}

/**
 * @brief   Print boot image information.
 *
 */
void app_ota_check_boot(void) {
#if CONFIG_ENABLE_LOGGING
    uint8_t         sha_256[HASH_LEN] = {0};
    esp_partition_t partition;

    // get sha256 digest for the partition table
    partition.address = ESP_PARTITION_TABLE_OFFSET;
    partition.size    = ESP_PARTITION_TABLE_MAX_LEN;
    partition.type    = ESP_PARTITION_TYPE_DATA;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for the partition table: ");

    // get sha256 digest for bootloader
    partition.address = ESP_BOOTLOADER_OFFSET;
    partition.size    = ESP_PARTITION_TABLE_OFFSET;
    partition.type    = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "SHA-256 for bootloader: ");

    // get sha256 digest for running partition
    const esp_partition_t* running = esp_ota_get_running_partition();
    esp_partition_get_sha256(running, sha_256);
    print_sha256(sha_256, "SHA-256 for current firmware: ");

    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        if (ota_state == ESP_OTA_IMG_NEW) {
            RTN_LOGI(TAG, "Running OTA IMG state: NEW");
        } else if (ota_state == ESP_OTA_IMG_PENDING_VERIFY) {
            RTN_LOGI(TAG, "Running OTA IMG state: PENDING_VERIFY");
        } else if (ota_state == ESP_OTA_IMG_VALID) {
            RTN_LOGI(TAG, "Running OTA IMG state: VALID");
        } else if (ota_state == ESP_OTA_IMG_INVALID) {
            RTN_LOGI(TAG, "Running OTA IMG state: INVALID");
        } else if (ota_state == ESP_OTA_IMG_ABORTED) {
            RTN_LOGI(TAG, "Running OTA IMG state: ABORTED");
        } else if (ota_state == ESP_OTA_IMG_UNDEFINED) {
            RTN_LOGI(TAG, "Running OTA IMG state: UNDEFINED");
        }
    }
#endif
}

/**
 * @brief   Initialize OTA driver.
 *
 * @param[out] ota_desc image descriptor pointer
 * @param[out] length   image lenght
 * @return              return msg
 *
 */
esp_err_t app_ota_init(const char* ota_desc, const uint16_t length) {
    RTN_LOGI(TAG, "Starting OTA update ..");

    esp_app_desc_t new_app_info;
    if (length != sizeof(esp_app_desc_t)) {
        RTN_LOGI(TAG, "Bad app description lenght");
        return ESP_FAIL;
    }
    memcpy(&new_app_info, ota_desc, sizeof(esp_app_desc_t));
    RTN_LOGI(TAG, "New firmware version: %s", new_app_info.version);

    const esp_partition_t* configured = esp_ota_get_boot_partition();
    const esp_partition_t* running    = esp_ota_get_running_partition();

    if (configured != running) {
        RTN_LOGW(TAG,
                 "Configured OTA boot partition at offset 0x%08x, but running "
                 "from offset 0x%08x",
                 configured->address, running->address);
        RTN_LOGW(TAG, "(This can happen if either the OTA boot data or "
                      "preferred boot image become corrupted somehow.)");
    }
    RTN_LOGI(TAG, "Running partition type %d subtype %d (offset 0x%08x)", running->type, running->subtype,
             running->address);

    update_partition = esp_ota_get_next_update_partition(NULL);
    if (update_partition == NULL) {
        return ESP_FAIL;
    } else {
        RTN_LOGI(TAG, "Writing to partition subtype %d at offset 0x%x", update_partition->subtype,
                 update_partition->address);
    }

    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
        RTN_LOGI(TAG, "Running firmware version: %s", running_app_info.version);
    }

    const esp_partition_t* last_invalid_app = esp_ota_get_last_invalid_partition();
    esp_app_desc_t         invalid_app_info;
    if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
        RTN_LOGI(TAG, "Last invalid firmware version: %s", invalid_app_info.version);
    }

    // check current version with last invalid partition
    if (last_invalid_app != NULL) {
        if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0) {
            RTN_LOGW(TAG, "New version is the same as invalid version.");
            RTN_LOGW(TAG,
                     "Previously, there was an attempt to launch the firmware "
                     "with %s version, but it failed.",
                     invalid_app_info.version);
            RTN_LOGW(TAG, "The firmware has been rolled back to the previous version.");
            return ESP_FAIL;
        }
    }

    if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) {
        RTN_LOGW(TAG, "Current running version is the same as a new. We will "
                      "not continue the update.");
        return ESP_FAIL;
    }

    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end()
     */
    update_handle      = 0;
    binary_file_length = 0;
    esp_err_t err      = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        RTN_LOGE(TAG, "esp_ota_begin failed (%s)", esp_err_to_name(err));
        return ESP_FAIL;
    }

    RTN_LOGI(TAG, "esp_ota_begin succeeded");
    return ESP_OK;
}

/**
 * @brief   Write OTA image.
 *
 * @param[out] ota_data image data pointer
 * @param[out] length   image lenght
 * @return     return msg
 *
 */
esp_err_t app_ota_write(const char* ota_data, const uint16_t length) {
    esp_err_t err = esp_ota_write(update_handle, (const void*)ota_data, length);
    if (err != ESP_OK) {
        return ESP_FAIL;
    } else {
        binary_file_length += length;
        RTN_LOGI(TAG, "Written image length %d", binary_file_length);
        return ESP_OK;
    }
}

/**
 * @brief   Update OTA image.
 *
 * @return  return msg
 *
 */
esp_err_t app_ota_update(void) {
    esp_err_t err = esp_ota_end(update_handle);

    if (err != ESP_OK) {
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            RTN_LOGE(TAG, "Image validation failed, image is corrupted");
        }
        RTN_LOGE(TAG, "esp_ota_end failed (%s)!", esp_err_to_name(err));
        return ESP_FAIL;
    } else {
        err = esp_ota_set_boot_partition(update_partition);
        if (err != ESP_OK) {
            RTN_LOGE(TAG, "esp_ota_set_boot_partition failed (%s)!", esp_err_to_name(err));
            return ESP_FAIL;
        } else {
            RTN_LOGI(TAG, "app_ota_update succeeded");
            return ESP_OK;
        }
    }
}

/** @} */
