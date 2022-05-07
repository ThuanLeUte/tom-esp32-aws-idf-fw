/**
* @file       sys_aws_shadow.h
* @copyright  Copyright (C) 2021 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2021-08-17
* @author     Thuan Le
* @brief      System module to handle Amazon Web Services Shadow (AWS)
* @note       None
* @example    None
*/

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __SYS_AWS_SHADOW_H
#define __SYS_AWS_SHADOW_H

/* Includes ----------------------------------------------------------- */
#include "stdbool.h"
#include "stdint.h"
#include "sys.h"
#include "aws_iot_shadow_interface.h"

/* Public defines ----------------------------------------------------- */
/* Public enumerate/structure ----------------------------------------- */
/**
 * @brief Shadow name enum
 */
typedef enum
{
   SYS_SHADOW_FIRMWARE_ID = 0
  ,SYS_SHADOW_SCALE_TARE
  ,SYS_SHADOW_MAX
}
sys_aws_shadow_name_t;

/**
 * @brief AWS shadown command enum
 */
typedef enum
{
   SYS_AWS_SHADOW_CMD_GET = 0
  ,SYS_AWS_SHADOW_CMD_SET
}
sys_aws_shadow_cmd_t;

/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */
/**
 * @brief         AWS shadow init
 *
 * @param[in]     None
 *
 * @attention     None
 *
 * @return        None
 */
bool sys_aws_shadow_init(void);

/**
 * @brief         AWS shadow trigger command
 *
 * @param[in]     cmd      Shadown command
 * @param[in]     name     Shadow name
 *
 * @attention     None
 *
 * @return        None
 */
void sys_aws_shadow_trigger_command(sys_aws_shadow_cmd_t cmd, sys_aws_shadow_name_t name);

/**
 * @brief         AWS shadow update
 *
 * @param[in]     name     Shadow name
 *
 * @attention     None
 *
 * @return
 *  - true:   Update success
 *  - false:  Update failed
 */
bool sys_aws_shadow_update(sys_aws_shadow_name_t name);

/**
 * @brief         AWS shadow get
 *
 * @param[in]     name   Shadow name
 *
 * @attention     None
 *
 * @return
 *  - true:   Get data success
 *  - false:ed
 */
bool sys_aws_shadow_get(sys_aws_shadow_name_t name);

#endif /* __SYS_AWS_SHADOW_H */

/* End of file -------------------------------------------------------- */
