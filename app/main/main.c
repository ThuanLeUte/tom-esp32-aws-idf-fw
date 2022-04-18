/**
* @file       main.c
* @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2020-07-17
* @author     Thuan Le
* @brief      Entry point
* @note       None
* @example    None
*/
/* Includes ----------------------------------------------------------------- */
#include "platform_common.h"
#include "sys.h"

/* Public defines ----------------------------------------------------------- */
/* Private enumerate/structure ---------------------------------------------- */
/* Private Constants -------------------------------------------------------- */
/* Private variables -------------------------------------------------------- */
/* Private macros ----------------------------------------------------------- */
/**
 * Entry point
 */
void app_main(void)
{
  sys_boot();

  while (1)
  {
    sys_run();
  }
}

/* End of file -------------------------------------------------------------- */
