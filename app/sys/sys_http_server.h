/**
 * @file       sys.h
 * @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
 * @license    This project is released under the Hydratech License.
 * @version    1.0.0
 * @date       2022-01-13
 * @author     Thuan Le
 * @brief      System file to handler http server component
 * @note       None
 * @example    None
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __SYS_HTTP_SERVER_H
#define __SYS_HTTP_SERVER_H

/* Includes ----------------------------------------------------------- */
#include "platform_common.h"

/* Public defines ----------------------------------------------------- */
/* Public enumerate/structure ----------------------------------------- */
/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */
void sys_http_server_start(void);
void sys_http_server_stop(void);

#endif // __SYS_HTTP_SERVER_H

/* End of file -------------------------------------------------------- */
