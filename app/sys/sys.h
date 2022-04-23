/**
 * @file       sys.h
 * @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
 * @license    This project is released under the Hydratech License.
 * @version    1.0.0
 * @date       2022-01-13
 * @author     Thuan Le
 * @brief      System file
 * @note       None
 * @example    None
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __SYS_H
#define __SYS_H

/* Includes ----------------------------------------------------------- */
#include "platform_common.h"

/* Public defines ----------------------------------------------------- */
#define DEVICE_FIRMWARE_VERSION           "10000000"
#define DEVICE_HARDWARE_VERSION           "100"

#define SYS_DEVCFG_DONE_EVT               BIT1

#define FSM_UPDATE_STATE(new_state)                     \
  do {                                                  \
    if (new_state < SYS_STATE_CNT) {                    \
      g_device.sys_state = new_state;                   \
      ESP_LOGW(TAG, "[FSM] new state: %s", #new_state); \
    }                                                   \
  } while (0)

/* Public enumerate/structure ----------------------------------------- */
/**
 * @brief System state machine enum
 */
typedef enum
{
  SYS_STATE_POWER_ON,
  SYS_STATE_NW_SETUP,
  SYS_STATE_READY,
  SYS_STATE_FACTORY_RESET,
  SYS_STATE_CNT
}
sys_fsm_state_t;

/**
 * @brief Device structure
 */
typedef struct
{
  sys_fsm_state_t sys_state;
}
sys_device_t;

/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
extern sys_device_t g_device;
extern EventGroupHandle_t g_sys_evt_group;

/* Public function prototypes ----------------------------------------- */
/**
 * @brief System boot
 */
void sys_boot(void);

/**
 * @brief System run
 */
void sys_run(void);

/**
 * @brief System event group set
 */
void sys_event_group_set(const EventBits_t bit_to_set);

/**
 * @brief System event group clear
 */
void sys_event_group_clear(const EventBits_t bit_to_clear);

#endif // __SYS_H

/* End of file -------------------------------------------------------- */
