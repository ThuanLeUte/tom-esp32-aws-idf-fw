/**
* @file       platform_common.h
* @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    1.0.0
* @date       2022-01-20
* @author     Thuan Le
* @brief      Platform common
* @note       None
* @example    None
*/
/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __PLATFORM_COMMON_H
#define __PLATFORM_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif
/* Includes ----------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// FreeRTOS
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "freertos/event_groups.h"

// ESP32
#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_event.h"
#include "driver/spi_master.h"

/* Public defines ----------------------------------------------------------- */
#define CHECK(expr, ret)               \
  do {                                 \
    if (!(expr)) {                     \
      ESP_LOGE("Error", "%s", #expr);  \
      return (ret);                    \
    }                                  \
  } while (0)

#define CHECK_STATUS(expr)              \
  do {                                  \
    base_status_t ret = (expr);         \
    if (BS_OK != ret) {                 \
      ESP_LOGE("Error", "%s", #expr);   \
      return (ret);                     \
    }                                   \
  } while (0)

#define _CONFIG_ENVIRONMENT_DEV         (1)
#define _CONFIG_ENVIRONMENT_PRODUCTION  (0)

/* Public enumerate/structure ----------------------------------------------- */
typedef enum
{
  BS_OK      = 0x00,
  BS_ERROR   = 0x01,
  BS_BUSY    = 0x02,
  BS_TIMEOUT = 0x03
}
base_status_t;

/* Public Constants --------------------------------------------------------- */
/* Public variables --------------------------------------------------------- */
/* Public macros ------------------------------------------------------------ */
/* Public APIs -------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
#ifdef __cplusplus
} // extern "C" {
#endif

#endif // __PLATFORM_COMMON_H

/* End of file -------------------------------------------------------------- */
