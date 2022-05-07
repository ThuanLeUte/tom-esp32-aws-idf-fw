/**
 * @file       sys.c
 * @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
 * @license    This project is released under the Hydratech License.
 * @version    1.0.0
 * @date       2022-01-13
 * @author     Thuan Le
 * @brief      System file
 * @note       None
 * @example    None
 */

/* Includes ----------------------------------------------------------- */
#include "sys.h"
#include "sys_wifi.h"
#include "sys_aws_provision.h"
#include "sys_nvs.h"
#include "sys_devcfg.h"
#include "bsp.h"
#include "sys_aws.h"
#include "sys_ota.h"

/* Private defines ---------------------------------------------------- */
static const char *TAG = "sys";

/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
/* Public variables --------------------------------------------------- */
sys_device_t g_device;
EventGroupHandle_t g_sys_evt_group;

/* Private variables -------------------------------------------------- */
/* Private Constants -------------------------------------------------- */
/* Private function prototypes ---------------------------------------- */
static void m_sys_evt_group_init(void);

/* Function definitions ----------------------------------------------- */
void sys_boot(void)
{
  sys_nvs_init();
  bsp_spiffs_init();
  m_sys_evt_group_init();
  sys_wifi_init();

  if (sys_wifi_is_configured())
  {
    sys_wifi_connect();

    // Check OTA enable or not
    if (g_nvs_setting_data.ota.enable)
      sys_ota_start();
    else
      FSM_UPDATE_STATE(SYS_STATE_READY);
  }
  else
  {
    // sys_devcfg_init();
    // FSM_UPDATE_STATE(SYS_STATE_NW_SETUP);
  }
}

void sys_run(void)
{
  switch (g_device.sys_state)
  {
  case SYS_STATE_POWER_ON:
  {
    bsp_delay_ms(1000);
    break;
  }

  case SYS_STATE_NW_SETUP:
  {
    bsp_delay_ms(1000);
    break;
  }
  
  case SYS_STATE_READY:
  {
    bsp_delay_ms(1000);

    break;
  }

  default:
    break;
  }
}

void sys_event_group_set(const EventBits_t bit_to_set)
{
  if (g_sys_evt_group != NULL)
    xEventGroupSetBits(g_sys_evt_group, bit_to_set);
}

void sys_event_group_clear(const EventBits_t bit_to_clear)
{
  if (g_sys_evt_group != NULL)
    xEventGroupClearBits(g_sys_evt_group, bit_to_clear);
}

/* Private function --------------------------------------------------------- */
/**
 * @brief System event group init
 */
static void m_sys_evt_group_init(void)
{
  g_sys_evt_group = xEventGroupCreate();
}
/* End of file -------------------------------------------------------- */