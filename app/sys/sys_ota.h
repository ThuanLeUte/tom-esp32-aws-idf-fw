/**
 * @file       sys_ota.h
 * @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
 * @license    This project is released under the Hydratech License.
 * @version    1.0.0
 * @date       2020-08-25
 * @author     Thuan Le
 * @brief      System module to handle OTA
 * @note       None
 * @example    None
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __SYS_OTA_H
#define __SYS_OTA_H

/* Includes ----------------------------------------------------------- */
#include "platform_common.h"
#include "esp_ota_ops.h"

/* Public defines ----------------------------------------------------- */
#define OTA_STATE_NONE          (0)
#define OTA_STATE_FAILED        (1)
#define OTA_STATE_SUCCEEDED     (2)

#define APP_STATE_NORMAL        (0)
#define APP_STATE_OTA           (1)

/* Public enumerate/structure ----------------------------------------- */
/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */
/**
 * @brief         System ota setup
 * 
 * @param[in]     http_url      Http url
 * 
 * @attention     None
 * 
 * @return        None
 */
void sys_ota_setup(const char *http_url);

/**
 * @brief         System ota update
 * 
 * @param[in]     http_url      Http url
 * 
 * @attention     None
 * 
 * @return
 *  - true:   Ota successfull
 *  - false:  Ota failed
 */
void sys_ota_start(void);

#endif /* __SYS_OTA_H */

/* End of file -------------------------------------------------------- */