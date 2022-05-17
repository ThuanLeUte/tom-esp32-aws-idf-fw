/**
 * @file       sys_time.h
 * @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
 * @license    This project is released under the Hydratech License.
 * @version    1.0.0
 * @date       2022-03-03
 * @author     Thuan Le
 * @brief      System file to handle system time
 * @note       None
 * @example    None
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __SYS_TIME_H
#define __SYS_TIME_H

/* Includes ----------------------------------------------------------- */
#include "platform_common.h"

/* Public defines ----------------------------------------------------- */
/* Public enumerate/structure ----------------------------------------- */
/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */
/**
 * @brief  System time init
 */
void sys_time_init(void);

/**
 * @brief  System time get epoch time (ms)
 */
void sys_time_get_epoch_ms(uint64_t *epoch);

#endif // __SYS_TIME_H

/* End of file -------------------------------------------------------- */