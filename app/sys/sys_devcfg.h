/**
* @file       sys_devcfg.h
* @copyright  Copyright (C) 2021 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2020-07-23
* @author     Thuan Le
* @brief      System module to handle device configurations using Blufi protocol 
* @note       None
* @example    None
*/
/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __SYS_DEVCFG_H
#define __SYS_DEVCFG_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ----------------------------------------------------------------- */
#include "platform_common.h"
#include "wifi_ssid_manager.h"
#include "blufi_security.h"
#include "sys.h"
#include "sys_nvs.h"
#include "esp_bt.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_ble_api.h"
#include "esp_blufi_api.h"
#include "sys_wifi.h"
#include "esp_blufi.h"

/* Public defines ----------------------------------------------------------- */
#define FLAG_QRCODE_NOT_SET                (0x00)
#define FLAG_QRCODE_SET                    (0x11)
#define FLAG_QRCODE_SET_SUCCESS            (0x22)

/* Public enumerate/structure ----------------------------------------------- */
/* Public Constants --------------------------------------------------------- */
/* Public variables --------------------------------------------------------- */
extern wifi_ssid_manager_handle_t g_ssid_manager;

/* Public macros ------------------------------------------------------------ */
/* Public APIs -------------------------------------------------------------- */
void sys_devcfg_init(void);

/**
 * @brief         Auto-connect to WiFi SSID based on its RSSI
 * 
 * @param[in]     None
 * 
 * @attention     None
 * 
 * @return        true
 * @return        false
 */
bool sys_devcfg_wifi_connect(void);

/* -------------------------------------------------------------------------- */
#ifdef __cplusplus
} // extern "C" {
#endif

#endif // __SYS_DEVCFG_H

/* End of file -------------------------------------------------------------- */
