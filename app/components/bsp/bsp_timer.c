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

/* Includes ----------------------------------------------------------------- */
#include "bsp_timer.h"
#include "bsp.h"

/* Private defines ---------------------------------------------------------- */
/* Private enumerate/structure ---------------------------------------------- */
/* Private Constants -------------------------------------------------------- */
/* Private variables -------------------------------------------------------- */
/* Private macros ----------------------------------------------------------- */
/* Private Constants -------------------------------------------------------- */
//static char *TAG = "bsp";

/* Private prototypes ------------------------------------------------------- */
static void m_bsp_tmr_start_ex(tmr_t *tm, tick_t start, tick_t interval);
static esp_err_t m_bsp_tmr_auto_start(esp_timer_handle_t handle, tick_t interval, esp_timer_cb_t callback);

/* Public APIs -------------------------------------------------------------- */
/**
 * @brief Start the simple timer
 */
void bsp_tmr_start(tmr_t *tm, tick_t interval)
{
  m_bsp_tmr_start_ex(tm, bsp_get_sys_tick_ms(), interval);
}

/**
 * @brief Stop the simple timer
 */
void bsp_tmr_stop(tmr_t *tm)
{
  tm->start    = 0;
  tm->interval = 0;
}

/**
 * @brief Restart the simple timer
 */
void bsp_tmr_restart(tmr_t *tm, tick_t interval)
{
  bsp_tmr_stop(tm);
  bsp_tmr_start(tm, interval);
}

bool bsp_tmr_is_expired(tmr_t *tm)
{
  if (tm->interval && (bsp_get_sys_tick_ms() - tm->start >= tm->interval))
  {
    bsp_tmr_stop(tm);
    return true;
  }
  else
  {
    return false;
  }
}

/**
 * @brief Initialize auto timer
 */
void bsp_tmr_auto_init(auto_timer_t *atm, esp_timer_cb_t callback)
{
  atm->timer.interval = 0;
  atm->timer.start    = 0;
  atm->callback       = callback;
}

/**
 * @brief Start auto timer
 */
void bsp_tmr_auto_start(auto_timer_t *atm, tick_t interval)
{
  atm->timer.interval = interval;
  bsp_tmr_auto_restart(atm);
}

/**
 * @brief Restart auto timer
 */
void bsp_tmr_auto_restart(auto_timer_t *atm)
{
  if (atm->timer.interval == 0) return;

  m_bsp_tmr_auto_start(atm->handle, atm->timer.interval, atm->callback);
}

/**
 * @brief Stop auto timer. Only called in non-IRQ routine/timer callback
 */
void bsp_tmr_auto_stop(auto_timer_t *atm)
{
  esp_timer_stop(atm->handle);
  esp_timer_delete(atm->handle);
  atm->handle = NULL;
}

/* Private function --------------------------------------------------------- */
/**
 * @brief Start the simple timer at exact start time
 */
static void m_bsp_tmr_start_ex(tmr_t *tm, tick_t start, tick_t interval)
{
  tm->start    = start;
  tm->interval = interval;
}

/**
 * @brief         Time start
 * 
 * @param[in]     type            System timer type
 * @param[in]     interval        Interval time (Ms)
 * @param[in]     level           Timer level
 * @param[in]     callback        Function callback
 * 
 * @return        ESP_OK
 * @return        ESP_FAILS
 */
static esp_err_t m_bsp_tmr_auto_start(esp_timer_handle_t handle,
                                      tick_t interval,
                                      esp_timer_cb_t callback)
{
  esp_timer_create_args_t timer_args =
  {
    .callback = callback,
    .arg      = NULL,
  };

  if (handle == NULL)
  {
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &handle));
    ESP_ERROR_CHECK(esp_timer_start_once(handle, interval * 1000));
  }

  return ESP_OK;
}

/* End of file -------------------------------------------------------------- */
