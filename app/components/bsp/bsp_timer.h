/**
* @file       bsp_timer.h
* @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2022-04-03
* @author     Thuan Le
* @brief      Timer driver
* @note       None
* @example    None
*/

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __BSP_TIMER_H
#define __BSP_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ----------------------------------------------------------------- */
#include "platform_common.h"

/* Public defines ----------------------------------------------------------- */
typedef uint32_t tick_t;    //!< Count of system tick

/* Public enumerate/structure ----------------------------------------------- */
/**
 * @brief Simple timer
 */
typedef struct
{
  tick_t start;
  tick_t interval;
}
tmr_t;

/**
 * @brief Auto timer structure
 */
typedef struct
{
  esp_timer_handle_t handle;
  tmr_t timer;
  esp_timer_cb_t callback;
}
auto_timer_t;

/* Public Constants --------------------------------------------------------- */
/* Public variables --------------------------------------------------------- */
/* Public macros ------------------------------------------------------------ */
/* Public APIs -------------------------------------------------------------- */
void bsp_tmr_start     (tmr_t *tm, tick_t interval);
void bsp_tmr_restart   (tmr_t *tm, tick_t interval);
void bsp_tmr_stop      (tmr_t *tm);
bool bsp_tmr_is_expired(tmr_t *tm);

void bsp_tmr_auto_init   (auto_timer_t *atm, esp_timer_cb_t callback);
void bsp_tmr_auto_start  (auto_timer_t *atm, tick_t interval);
void bsp_tmr_auto_restart(auto_timer_t *atm);
void bsp_tmr_auto_stop   (auto_timer_t *atm);

/* -------------------------------------------------------------------------- */
#ifdef __cplusplus
} // extern "C" {
#endif

#endif // __BSP_TIMER_H

/* End of file ---------------------------------------------------------------- */
