/**
* @file       sys_aws_jobs.h
* @copyright  Copyright (C) 2021 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2021-08-21
* @author     Thuan Le
* @brief      System module to handle Amazon Web Services Jobs (AWS)
* @note       None
* @example    None
*/

/* Define to prevent recursive inclusion ------------------------------------ */
#ifndef __SYS_AWS_JOBS_H
#define __SYS_AWS_JOBS_H

/* Includes ----------------------------------------------------------------- */
#include "sys_aws_config.h"
#include "platform_common.h"

#include "aws_iot_config.h"
#include "aws_iot_json_utils.h"
#include "aws_iot_jobs_interface.h"
#include "aws_iot_mqtt_client_interface.h"

/* Private enum/structs ----------------------------------------------------- */
/* Private defines ---------------------------------------------------------- */
/* Private Constants -------------------------------------------------------- */
static const char *TAG_JOB = "sys/aws_jobs";

/* Private variables -------------------------------------------------------- */
static const char     *m_thing_name;
static AWS_IoT_Client *m_client;

/* Public variables --------------------------------------------------------- */
/* Function prototypes ------------------------------------------------------ */
void sys_aws_jobs_init(AWS_IoT_Client *p_client, const char *thing_name, pApplicationHandler_t p_next_job_cb);

/* Private function prototypes ---------------------------------------------- */
static void m_sys_aws_jobs_update_accepted_callback(AWS_IoT_Client *p_client,
                                                    char *topic_name,
                                                    uint16_t topic_name_len,
                                                    IoT_Publish_Message_Params *params,
                                                    void *p_data);

static void m_sys_aws_jobs_update_rejected_callback(AWS_IoT_Client *p_client,
                                                    char *topic_name,
                                                    uint16_t topic_name_len,
                                                    IoT_Publish_Message_Params *params,
                                                    void *p_data);

/* Function definitions ----------------------------------------------------- */
void sys_aws_jobs_send_update(const char *job_id, JobExecutionStatus status)
{
  AwsIotJobExecutionUpdateRequest update_request;
  char topic_to_publish_update[MAX_JOB_TOPIC_LENGTH_BYTES];
  char message_buffer[200];
  IoT_Error_t rc = FAILURE;

  update_request.status                   = status;
  update_request.statusDetails            = NULL;
  update_request.expectedVersion          = 0;
  update_request.executionNumber          = 0;
  update_request.includeJobExecutionState = false;
  update_request.includeJobDocument       = false;
  update_request.clientToken              = NULL;

  rc = aws_iot_jobs_send_update(m_client, QOS0, m_thing_name, job_id, &update_request,
                                topic_to_publish_update, sizeof(topic_to_publish_update), message_buffer, sizeof(message_buffer));

  ESP_LOGW(TAG_JOB, "Error: %d, aws_iot_jobs_send_update: %s", rc, topic_to_publish_update);
}

inline void __attribute__((always_inline)) sys_aws_jobs_init(AWS_IoT_Client *p_client, const char *thing_name, pApplicationHandler_t p_next_job_cb)
{
  m_thing_name = thing_name;
  m_client     = p_client;

  char topic_to_subscribe_notify_next[MAX_JOB_TOPIC_LENGTH_BYTES];
  char topic_to_subscribe_get_next[MAX_JOB_TOPIC_LENGTH_BYTES];
  char topic_to_subscribe_update_accepted[MAX_JOB_TOPIC_LENGTH_BYTES];
  char topic_to_subscribe_update_rejected[MAX_JOB_TOPIC_LENGTH_BYTES];

  char topic_to_publish_get_next[MAX_JOB_TOPIC_LENGTH_BYTES];

  AwsIotDescribeJobExecutionRequest describe_request;
  describe_request.executionNumber    = 0;
  describe_request.includeJobDocument = true;
  describe_request.clientToken        = NULL;

  IoT_Error_t rc = FAILURE;

  rc = aws_iot_jobs_subscribe_to_job_messages(p_client, QOS0, thing_name,
                                              NULL, JOB_NOTIFY_NEXT_TOPIC, JOB_REQUEST_TYPE,
                                              p_next_job_cb, NULL, topic_to_subscribe_notify_next,
                                              sizeof(topic_to_subscribe_notify_next));

  ESP_LOGW(TAG_JOB, "Error: %d, JOB_NOTIFY_NEXT_TOPIC  : %s", rc, topic_to_subscribe_notify_next);

  rc = aws_iot_jobs_subscribe_to_job_messages(p_client, QOS0, thing_name,
                                              JOB_ID_NEXT, JOB_DESCRIBE_TOPIC, JOB_WILDCARD_REPLY_TYPE,
                                              p_next_job_cb, NULL, topic_to_subscribe_get_next,
                                              sizeof(topic_to_subscribe_get_next));

  ESP_LOGW(TAG_JOB, "Error: %d, JOB_DESCRIBE_TOPIC     : %s", rc, topic_to_subscribe_get_next);

  rc = aws_iot_jobs_subscribe_to_job_messages(p_client, QOS0, thing_name,
                                              JOB_ID_WILDCARD, JOB_UPDATE_TOPIC, JOB_ACCEPTED_REPLY_TYPE,
                                              m_sys_aws_jobs_update_accepted_callback, NULL, topic_to_subscribe_update_accepted,
                                              sizeof(topic_to_subscribe_update_accepted));

  ESP_LOGW(TAG_JOB, "Error: %d, JOB_UPDATE_TOPIC       : %s", rc, topic_to_subscribe_update_accepted);

  rc = aws_iot_jobs_subscribe_to_job_messages(p_client, QOS0, thing_name,
                                              JOB_ID_WILDCARD, JOB_UPDATE_TOPIC, JOB_REJECTED_REPLY_TYPE,
                                              m_sys_aws_jobs_update_rejected_callback, NULL, topic_to_subscribe_update_rejected,
                                              sizeof(topic_to_subscribe_update_rejected));

  ESP_LOGW(TAG_JOB, "Error: %d, JOB_UPDATE_TOPIC       : %s", rc, topic_to_subscribe_update_rejected);

  rc = aws_iot_jobs_describe(p_client, QOS0, thing_name,
                             JOB_ID_NEXT, &describe_request, topic_to_publish_get_next,
                             sizeof(topic_to_publish_get_next), NULL, 0);

  ESP_LOGW(TAG_JOB, "Error: %d, JOB_DESCRIBE_TOPIC  : %s", rc, topic_to_publish_get_next);

  aws_iot_mqtt_yield(p_client, 1000);
}

/* Private function definitions --------------------------------------------- */
/**
 * @brief         AWS jobs update accepted callback
 *
 * @param[in]     p_client        Pointer to aws iot client
 * @param[in]     topic_name      Pointer to topic name
 * @param[in]     topic_name_len  Topic name length
 * @param[in]     params          Pointer to params
 * @param[in]     p_data          Pointer to data
 *
 * @attention     None
 *
 * @return        None
 */
static void m_sys_aws_jobs_update_accepted_callback(AWS_IoT_Client *p_client,
                                                    char *topic_name,
                                                    uint16_t topic_name_len,
                                                    IoT_Publish_Message_Params *params,
                                                    void *p_data)

{
  IOT_UNUSED(p_data);
  IOT_UNUSED(p_client);
  ESP_LOGW(TAG_JOB, "AWS jobs update accepted callback");
  ESP_LOGI(TAG_JOB, "Topic: %.*s", topic_name_len, topic_name);
  ESP_LOGI(TAG_JOB, "Payload: %.*s", (int)params->payloadLen, (char *)params->payload);
}

/**
 * @brief         AWS jobs update rejected callback
 *
 * @param[in]     p_client        Pointer to aws iot client
 * @param[in]     topic_name      Pointer to topic name
 * @param[in]     topic_name_len  Topic name length
 * @param[in]     params          Pointer to params
 * @param[in]     p_data          Pointer to data
 *
 * @attention     None
 *
 * @return        None
 */
static void m_sys_aws_jobs_update_rejected_callback(AWS_IoT_Client *p_client,
                                                    char *topic_name,
                                                    uint16_t topic_name_len,
                                                    IoT_Publish_Message_Params *params,
                                                    void *p_data)
{
  IOT_UNUSED(p_data);
  IOT_UNUSED(p_client);
  ESP_LOGW(TAG_JOB, "AWS jobs update rejected callback");
  ESP_LOGI(TAG_JOB, "Topic: %.*s", topic_name_len, topic_name);
  ESP_LOGI(TAG_JOB, "Payload: %.*s", (int)params->payloadLen, (char *)params->payload);
}

#endif /* __SYS_AWS_JOBS_H */

/* End of file -------------------------------------------------------- */