/**
* @file       sys_aws_mqtt.c
* @copyright  Copyright (C) 2021 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2022-02-22
* @author     Thuan Le
* @brief      System module to handle Amazon Web Services MQTT (AWS)
* @note       None
* @example    None
*/

/* Includes ----------------------------------------------------------------- */
#include "platform_common.h"
#include "sys_aws_mqtt.h"
#include "sys_aws_config.h"
#include "sys_aws.h"
#include "sys_nvs.h"

#include "aws_iot_mqtt_client_interface.h"
#include "jsmn.h"
#include "aws_iot_json_utils.h"
#include "frozen.h"

/* Private enum/structs ----------------------------------------------------- */
/* Private defines ---------------------------------------------------------- */
#define AWS_PUB_MSG_SIZE_MAX              (1000) // Size max of json data for publish payload

static const char *AWS_SUBSCRIBE_TOPIC[] =
{
  "covid19/"AWS_WORKING_ENVIRONMENT"/%s/down",
};

static const char *AWS_PUBLISH_TOPIC[] =
{
  "covid19/"AWS_WORKING_ENVIRONMENT"/%s/up",
};

/* Private Constants -------------------------------------------------------- */
static const char *TAG = "sys/aws_mqtt";

/* Private variables -------------------------------------------------------- */
static jsmn_parser  m_json_parser;
static jsmntok_t    m_json_token_struct[MAX_JSON_TOKEN_EXPECTED];

/* Public variables --------------------------------------------------------- */
/* Private function prototypes ---------------------------------------------- */
static void m_sys_aws_subscribe_callback_handler(AWS_IoT_Client             *p_client,
                                                 char                       *topic_name,
                                                 uint16_t                   topic_name_len,
                                                 IoT_Publish_Message_Params *params,
                                                 void                       *p_data);

/* Function definitions ----------------------------------------------------- */
// void sys_aws_mqtt_send_noti(aws_noti_type_t noti_type, void *param1, void *param2)
// {
//   g_sys_aws.service.mqtt.data.packet_type                = AWS_PKT_NOTI;
//   g_sys_aws.service.mqtt.data.noti_param.noti_type       = noti_type;
//   g_sys_aws.service.mqtt.data.noti_param.noti_id         = 1;
//   g_sys_aws.service.mqtt.data.noti_param.info.time       = bsp_rtc_get_time();
//   strcpy(g_sys_aws.service.mqtt.data.noti_param.info.patient_id, g_nvs_setting_data.patient_id);

//   sys_aws_mqtt_trigger_publish(AWS_PUB_TOPIC_UPSTREAM);
// }

void sys_aws_mqtt_trigger_publish(sys_aws_mqtt_pub_topic_t topic)
{
  g_sys_aws.service.type           = SYS_AWS_MQTT;
  g_sys_aws.service.mqtt.cmd       = SYS_AWS_MQTT_CMD_PUB;
  g_sys_aws.service.mqtt.pub_topic = topic;

  xQueueSend(g_sys_aws.evt_queue, (void *)&g_sys_aws.service, pdMS_TO_TICKS(100));
}

bool sys_aws_mqtt_subscribe(sys_aws_mqtt_sub_topic_t topic)
{
  IoT_Error_t rc;

  static char aws_sub_topic[100];

  sprintf(aws_sub_topic, AWS_SUBSCRIBE_TOPIC[topic], g_nvs_setting_data.thing_name);

  ESP_LOGI(TAG, "Subscribing...: %s", aws_sub_topic);

  rc = aws_iot_mqtt_subscribe(&g_sys_aws.client, aws_sub_topic, strlen(aws_sub_topic), QOS0,
                              m_sys_aws_subscribe_callback_handler, NULL);

  if (SUCCESS != rc)
  {
    ESP_LOGI(TAG, "Error subscribing : %d ", rc);
    return false;
  }

  return true;
}

bool sys_aws_mqtt_publish(sys_aws_mqtt_pub_topic_t topic, char *buf)
{
  IoT_Error_t rc;
  IoT_Publish_Message_Params params_publish_msg;

  static char aws_pub_topic[100];

  sprintf(aws_pub_topic, AWS_PUBLISH_TOPIC[topic], g_nvs_setting_data.thing_name);

  params_publish_msg.payload    = (void *)buf;
  params_publish_msg.payloadLen = strlen(buf);

  ESP_LOGI(TAG, "Publishing...: %s", aws_pub_topic);

  rc = aws_iot_mqtt_publish(&g_sys_aws.client, aws_pub_topic, strlen(aws_pub_topic), &params_publish_msg);

  if (SUCCESS != rc)
  {
    ESP_LOGI(TAG, "Error publishing : %d ", rc);
    return false;
  }

  return true;
}

/* Private function definitions --------------------------------------------- */
/**
 * @brief         AWS subscibe callback handler
 *
 * @param[in]     p_client        Pointer to client
 * @param[in]     topic_name      Pointer to topic name
 * @param[in]     topic_name_len  Topic name length
 * @param[in]     params          Pointer to params
 * @param[in]     p_data          Pointer to data
 *
 * @attention     None
 *
 * @return        None
 */
static void m_sys_aws_subscribe_callback_handler(AWS_IoT_Client *p_client,
                                                 char *topic_name,
                                                 uint16_t topic_name_len,
                                                 IoT_Publish_Message_Params *params,
                                                 void *p_data)
{
  jsmntok_t *json_obs;
  char buf[200];
  memset(buf, 0, 200);

  ESP_LOGI(TAG, "Subscribe callback");
  ESP_LOGI(TAG, "%.*s\t%.*s", topic_name_len, topic_name, (int)params->payloadLen, (char *)params->payload);

  // Json parse data that contains certificate data
  jsmn_init(&m_json_parser);
  jsmn_parse(&m_json_parser,
             (char *)params->payload,
             (int)params->payloadLen,
             m_json_token_struct,
             sizeof(m_json_token_struct) / sizeof(m_json_token_struct[0]));

  // Check "type"
  json_obs = findToken("type", params->payload, m_json_token_struct);
  if (json_obs)
  {
    memcpy(buf, params->payload + json_obs->start, json_obs->end - json_obs->start);
    ESP_LOGI(TAG, "type: %s", buf);

    if (0 == strcmp("req", buf))
    {
      json_obs = findToken("get_dev_info", params->payload, m_json_token_struct);
      if (json_obs)
      {
        // sys_aws_mqtt_send_response(AWS_REQ_GET_DEVICE_INFO);
      }
    }
    else if (0 == strcmp("ack", buf))
    {
      json_obs = findToken("nt", params->payload, m_json_token_struct);
      if (json_obs)
      {
        // TODO: Allow new data can send out
      }
    }
  }
}

/* End of file -------------------------------------------------------------- */