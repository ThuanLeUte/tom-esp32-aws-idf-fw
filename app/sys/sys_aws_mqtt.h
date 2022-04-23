/**
* @file       sys_aws_mqtt.h
* @copyright  Copyright (C) 2021 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2021-08-13
* @author     Thuan Le
* @brief      System module to handle Amazon Web Services MQTT (AWS)
* @note       None
* @example    None
*/

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __SYS_AWS_MQTT_H
#define __SYS_AWS_MQTT_H

/* Includes ----------------------------------------------------------- */
#include "platform_common.h"

/* Public defines ----------------------------------------------------- */
/* Public enumerate/structure ----------------------------------------- */
/**
 * @brief AWS MQTT command enum
 */
typedef enum
{
   SYS_AWS_MQTT_CMD_PUB = 0
  ,SYS_AWS_MQTT_CMD_SUB
}
sys_aws_mqtt_cmd_t;

/**
 * AWS publish topics
 */
typedef enum
{
  AWS_PUB_TOPIC_UPSTREAM
}
sys_aws_mqtt_pub_topic_t;

/**
 * AWS subcribe topics
 */
typedef enum
{
  AWS_PUB_TOPIC_DOWNNSTREAM
}
sys_aws_mqtt_sub_topic_t;

/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */
/**
 * @brief         AWS MQTT trigger publish
 *
 * @param[in]     topic   Topic to be published
 *
 * @attention     None
 *
 * @return        None
 */
void sys_aws_mqtt_trigger_publish(sys_aws_mqtt_pub_topic_t topic);

/**
 * @brief         AWS MQTT subscribe
 *
 * @param[in]     None
 *
 * @attention     None
 *
 * @return        None
 */
bool sys_aws_mqtt_subscribe(sys_aws_mqtt_sub_topic_t topic);

/**
 * @brief         AWS MQTT publish
 *
 * @param[in]     topic   Topic to be published
 *
 * @attention     None
 *
 * @return        None
 */
bool sys_aws_mqtt_publish(sys_aws_mqtt_pub_topic_t topic, char *buf);

// void sys_aws_mqtt_send_noti(aws_noti_type_t noti_type, void *param1, void *param2);

#endif /* __SYS_AWS_MQTT_H */

/* End of file -------------------------------------------------------- */