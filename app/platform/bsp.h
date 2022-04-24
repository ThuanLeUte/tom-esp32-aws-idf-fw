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
typedef enum
{
  BSP_ERR_NONE

  //                                            |-------- 1: On-chip, 2: Off-chip
  //                                             ||------ 00 - 99: Peripheral type
  //                                               ||---- 00 - 99: Error code
  ,BSP_ERR_SD_INIT                            = 20000
  ,BSP_ERR_SD_COMMUNICATION                   = 20001
  
  ,BSP_ERR_IOE_INIT                           = 20100
  ,BSP_ERR_IOE_COMMUNICATION                  = 20101

  ,BSP_ERR_TEMPERATURE_INIT                   = 20200
  ,BSP_ERR_TEMPERATURE_COMMUNICATION          = 20201

  ,BSP_ERR_RTC_INIT                           = 20300
  ,BSP_ERR_RTC_COMMUNICATION                  = 20301

  ,BSP_ERR_NVS_INIT                           = 20400
  ,BSP_ERR_NVS_COMMUNICATION                  = 20401
}
bsp_error_t;

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
void bsp_error_handler(bsp_error_t error);

void bsp_delay_ms(uint32_t ms);

void bsp_spiffs_init(void);

/* -------------------------------------------------------------------------- */
#ifdef __cplusplus
} // extern "C" {
#endif

#endif // __BSP_H

/* End of file ---------------------------------------------------------------- */
