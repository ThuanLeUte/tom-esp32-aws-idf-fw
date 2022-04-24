/**
* @file       sys_aws_config.h
* @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2020-12-24
* @author     Thuan Le
* @brief      AWS file config
* @note       None
* @example    None
*/

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __SYS_AWS_CONFIG_H
#define __SYS_AWS_CONFIG_H

/* Includes ----------------------------------------------------------- */
#include "platform_common.h"

#define AWS_HOST_ADDRESS                "aflobc52fa8dv-ats.iot.us-east-1.amazonaws.com"

#if (_CONFIG_ENVIRONMENT_DEV)
#define AWS_TEMPLATE_NAME                "esp32-dev-provisioning-template"
#define AWS_WORKING_ENVIRONMENT          "dev"
#endif

#if (_CONFIG_ENVIRONMENT_PRODUCTION)
#define AWS_TEMPLATE_NAME                "esp32-prod-provisioning-template"
#define AWS_WORKING_ENVIRONMENT          "production"
#endif

#define AWS_OFFICIAL_CERTIFICATE_PATH    "/spiffs/certificate.pem.crt"
#define AWS_OFFICIAL_PRIVATE_KEY_PATH    "/spiffs/private.pem.key"
#define AWS_PORT                         (8883)

/* Public defines ----------------------------------------------------- */
/* Public enumerate/structure ----------------------------------------- */
/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */

#endif /* __SYS_AWS_CONFIG_H */

/* End of file -------------------------------------------------------- */