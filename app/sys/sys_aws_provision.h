/**
* @file       sys_aws_provision.h
* @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2020-04-18
* @author     Thuan Le
* @brief      System module to handle Amazon Web Services Provision (AWS)
* @note       None
* @example    None
*/

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __SYS_AWS_PROVISION_H
#define __SYS_AWS_PROVISION_H

/* Includes ----------------------------------------------------------- */
#include "stdbool.h"
#include "stdint.h"

/* Public defines ----------------------------------------------------- */
#define AWS_PROVISION_NONE              (0x11)
#define AWS_PROVISION_DONE              (0x22)

/* Public enumerate/structure ----------------------------------------- */
/**
 * @brief AWS Provision Success Callback
 */
typedef void (*aws_provision_success_callback)(const char *thing_name);

/**
 * @brief AWS Provision Initialization Parameters
 */
typedef struct
{
  char aws_client_id[32];
  uint16_t aws_client_id_len;
  char qr_code[50];
  uint16_t qr_code_len;
  aws_provision_success_callback provision_success_cb;
}
sys_aws_provision_init_params_t;

/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */
/**
 * @brief         AWS start provision
 *
 * @param[in]     None
 *
 * @attention     None
 *
 * @return        None
 */
void sys_aws_provision_start(sys_aws_provision_init_params_t *params);

#endif /* __SYS_AWS_PROVISION_H */

/* End of file -------------------------------------------------------- */
