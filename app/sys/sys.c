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

/* Private defines ---------------------------------------------------- */
static const char *TAG = "sys";

/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
/* Public variables --------------------------------------------------- */
/* Private variables -------------------------------------------------- */
/* Private function prototypes ---------------------------------------- */
/* Private Constants -------------------------------------------------- */
/* Function definitions ----------------------------------------------- */
void sys_boot(void)
{
  sys_nvs_init();
  sys_wifi_init();
  sys_wifi_connect();
}

void sys_run(void)
{
  vTaskDelay(pdMS_TO_TICKS(1000));
}

/* End of file -------------------------------------------------------- */