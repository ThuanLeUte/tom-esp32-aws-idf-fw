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

/* Public enumerate/structure ----------------------------------------- */
/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */
/**
 * @brief         Start task handle IO
 */
void sys_wifi_init(void);

/**
 * @brief         Wifi connect
 */
void sys_wifi_connect(void);

/**
 * @brief         Check that wifi is connected or not
 */
bool sys_wifi_is_connected(void);

/**
 * @brief         Erase wifi config
 */
void sys_wifi_erase_config(void);

/**
 * @brief         Check that wifi is configured or not
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

#endif /* __SYS_WIFI_H */

/* End of file -------------------------------------------------------- */
