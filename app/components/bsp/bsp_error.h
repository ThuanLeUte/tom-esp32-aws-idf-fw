/**
 * @file       bsp_error.h
 * @copyright  Copyright (C) 2021 Hydratech. All rights reserved.
 * @license    This project is released under the Hydratech License.
 * @version    1.0.0
 * @date       2022-01-24
 * @author     Thuan Le
 * @brief      Board Support Error Handler
 * @note       None
 * @example    None
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __BSP_ERROR_H
#define __BSP_ERROR_H

/* Includes ----------------------------------------------------------- */
#include "platform_common.h"

/* Public defines ----------------------------------------------------- */
#define BSP_ERROR_CNT_MAX         100

/* Public variables --------------------------------------------------- */
typedef struct
{
  struct
  {
    uint32_t code[BSP_ERROR_CNT_MAX];
    uint16_t err_idx;
    uint16_t err_cnt;
  }
  nvs;

  uint16_t err_start;
}
bsp_error_t;

/* Public enumerate/structure ----------------------------------------- */
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
bsp_error_code_t;

/* Public function prototypes ----------------------------------------- */
/**
 * @brief         Read error data in flash
 * 
 * @param[in]     None
 * 
 * @return        None
 */
void bsp_error_init(void);

/**
 * @brief         This function is executed in case of error occurrence.
 * 
 * @param[in]     err   Error
 * 
 * @return        None
 */
void bsp_error_add(bsp_error_code_t err);

/**
 * @brief         Remove error in database
 * 
 * @param[in]     None
 * 
 * @return        None
 */
void bsp_error_remove(void);

/**
 * @brief         Calculate error start read position
 * 
 * @param[in]     None
 * 
 * @return        Numer of error in database
 */
uint16_t bsp_error_read_start(void);

/**
 * @brief         Read error
 * 
 * @param[in]     None
 * 
 * @return        Error code
 */
uint32_t bsp_error_read(void);

/**
 * @brief         Save error data to flash
 * 
 * @param[in]     None
 * 
 * @return        None
 */
void bsp_error_sync(void);

#endif /* __BSP_ERROR_H */

/* End of file -------------------------------------------------------- */