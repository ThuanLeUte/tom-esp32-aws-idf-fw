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

/* Includes ----------------------------------------------------------------- */
#include "bsp.h"
#include "platform_common.h"

/* Private defines ---------------------------------------------------------- */
/* Private enumerate/structure ---------------------------------------------- */
/* Private Constants -------------------------------------------------------- */
/* Private variables -------------------------------------------------------- */
/* Private macros ----------------------------------------------------------- */
/* Private Constants -------------------------------------------------------- */
static char *TAG = "bsp";

/* Private prototypes ------------------------------------------------------- */
/* Public APIs -------------------------------------------------------------- */
void bsp_error_handler(bsp_error_t error)
{
  ESP_LOGE(TAG, "BS_ERROR: %d", error);
}

/* Private function --------------------------------------------------------- */
/* End of file -------------------------------------------------------------- */
