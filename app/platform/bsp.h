/**
* @file       bsp.h
* @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2022-24-01
* @author     Thuan Le
* @brief      Board Support Packages
* @note       None
* @example    None
*/

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __BSP_H
#define __BSP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ----------------------------------------------------------------- */
#include "platform_common.h"

/* Public defines ----------------------------------------------------------- */
/* Public enumerate/structure ----------------------------------------------- */
/* Public Constants --------------------------------------------------------- */
/* Public variables --------------------------------------------------------- */
/* Public macros ------------------------------------------------------------ */
/* Public APIs -------------------------------------------------------------- */
/**
 * @brief         This function is executed in case of error occurrence.
 * 
 * @param[in]     error    Error
 * 
 * @return        None
 */
void bsp_delay_ms(uint32_t ms);

void bsp_spiffs_init(void);

uint32_t bsp_get_sys_tick_ms(void);

/* -------------------------------------------------------------------------- */
#ifdef __cplusplus
} // extern "C" {
#endif

#endif // __BSP_H

/* End of file ---------------------------------------------------------------- */
