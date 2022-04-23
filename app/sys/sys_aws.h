/**
* @file       sys_aws.h
* @copyright  Copyright (C) 2021 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2021-08-13
* @author     Thuan Le
* @brief      System module to handle Amazon Web Services Common(AWS)
* @note       None
* @example    None
*/

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __SYS_AWS_H
#define __SYS_AWS_H

/* Includes ----------------------------------------------------------- */
#include "platform_common.h"
#include "sys_aws_shadow.h"
#include "sys_aws_mqtt.h"
#include "aws_builder.h"
#include "aws_parser.h"

/* Public defines ----------------------------------------------------- */
#define FLAG_QRCODE_NOT_SET                (0x00)
#define FLAG_QRCODE_SET                    (0x11)
#define FLAG_QRCODE_SET_SUCCESS            (0x22)

/* Public enumerate/structure ----------------------------------------- */
/**
 * @brief AWS service type enum
 */
typedef enum
{
  SYS_AWS_SHADOW,
  SYS_AWS_MQTT
}
sys_aws_service_type_t;

/**
 * @brief AWS service and data structure
 */
typedef struct
{
  struct
  {
    sys_aws_shadow_cmd_t cmd;
    sys_aws_shadow_name_t name;

    struct
    {
      char firmware_id[10];
      uint8_t schedule_type;
    }
    data;
  }
  shadow;
  
  struct
  {
    sys_aws_mqtt_cmd_t cmd;
    sys_aws_mqtt_pub_topic_t pub_topic;
    struct
    {
      aws_packet_type_t packet_type;
      aws_resp_param_t resp_param; 
      aws_noti_param_t noti_param;
    }
    data;
  }
  mqtt;

  sys_aws_service_type_t type;
}
sys_aws_service_t;

/* Public macros ------------------------------------------------------ */
typedef struct
{
  AWS_IoT_Client client;
  QueueHandle_t evt_queue;
  sys_aws_service_t service;
  bool initialized;
}
sys_aws_t;

/* Public variables --------------------------------------------------- */
extern sys_aws_t g_sys_aws;

/* Public function prototypes ----------------------------------------- */
/**
 * @brief         AWS init
 *
 * @param[in]     None
 *
 * @attention     None
 *
 * @return        None
 */
void sys_aws_init(void);

/**
 * @brief         AWS start pub sub
 *
 * @param[in]     None
 *
 * @attention     None
 *
 * @return        None
 */
void sys_aws_start(void);

#endif /* __SYS_AWS_H */

/* End of file -------------------------------------------------------- */