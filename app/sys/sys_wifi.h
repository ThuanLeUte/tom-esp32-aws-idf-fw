/**
 * @file       sys_wifi.h
 * @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
 * @license    This project is released under the Hydratech License.
 * @version    1.0.0
 * @date       2022-02-18
 * @author     Thuan Le
 * @brief      System module to handle WiFi
 * @note       None
 * @example    None
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __SYS_WIFI_H
#define __SYS_WIFI_H

/* Includes ----------------------------------------------------------- */
#include "platform_common.h"

/* Public defines ----------------------------------------------------- */
#define WIFI_CONNECT_TIMEOUT_MS         (20000)     // 20s
#define WIFI_MAX_STATION_NUM            (100)       // Max wifi stations store in flash

#define ESP_WIFI_SSID_DEFAULT_AP        "LOX_SCALE"
#define ESP_WIFI_PASS_DEFAULT_AP        "123456789"

/* Public enumerate/structure ----------------------------------------- */
typedef enum
{
  SYS_WIFI_MODE_STA = 0,
  SYS_WIFI_MODE_AP
}
sys_wifi_mode_t;

/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */
/**
 * @brief         Init WiFi
 */
void sys_wifi_init(void);

/**
 * @brief         Init WiFi station
 */
void sys_wifi_sta_init(void);

/**
 * @brief         Init WiFi softap
 */
void sys_wifi_softap_init(void);

/**
 * @brief         Wifi connect
 */
void sys_wifi_sta_start(void);

bool sys_wifi_connect(const char *ssid, const char *password);
bool sys_wifi_is_scan_done(void);;
void sys_wifi_get_scan_wifi_list(char *buf, uint16_t size);
void sys_wifi_set_wifi_scan_status(bool done);

/**
 * @brief         Check that WiFi is connected or not
 */
bool sys_wifi_is_connected(void);

/**
 * @brief         Erase WiFi config
 */
void sys_wifi_erase_config(void);

/**
 * @brief         Check that WiFi is configured or not
 */
bool sys_wifi_is_configured(void);

/**
 * @brief         Set connection status
 */
void sys_wifi_set_connection_status(bool status);

/**
 * @brief         Update event handler
 */
void sys_wifi_update_event_handler(void);

/**
 * @brief         WiFi config set
 * 
 */
void sys_wifi_config_set(const char *ssid, const char *pwd);

void sys_wifi_scan_start(void);

#endif /* __SYS_WIFI_H */

/* End of file -------------------------------------------------------- */
