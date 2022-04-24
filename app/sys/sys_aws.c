/**
* @file       sys_aws.c
* @copyright  Copyright (C) 2021 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2021-08-13
* @author     Thuan Le
* @brief      System module to handle Amazon Web Services Common(AWS)
* @note       None
* @example    None
*/

/* Includes ----------------------------------------------------------------- */
#include "sys_aws_config.h"
#include "sys_aws.h"
#include "sys_aws_job.h"
#include "sys_aws_provision.h"
#include "sys_devcfg.h"

#include "platform_common.h"
#include "aws_iot_config.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "jsmn.h"
#include "aws_iot_json_utils.h"
#include "frozen.h"

/* Private enum/structs ----------------------------------------------------- */
static const char *AWS_JOB_OPERATION[] =
{
  "firmware_upgrade",
  "factory_reset"
};

typedef enum
{
  AWS_DFU_JOB,
  AWS_FACTORY_RESET_JOB
}
aws_job_t;

/* Private defines ---------------------------------------------------------- */
#define AWS_TASK_STACK_SIZE           (8192 / sizeof(StackType_t))
#define AWS_TASK_PRIORITY             (3)

#define MAX_SIZE_OF_JOB_OPERATION     (20)
#define MAX_SIZE_OF_JOB_UPGRADE_URL   (150)

/* Private Constants -------------------------------------------------------- */
static const char *TAG = "sys/aws";

/* Public variables --------------------------------------------------------- */
sys_aws_t g_sys_aws;

/* Private variables -------------------------------------------------------- */
static jsmn_parser    m_json_parser;
static jsmntok_t      m_json_token_struct[MAX_JSON_TOKEN_EXPECTED];
static int32_t        m_token_count;

static const uint8_t aws_root_ca_pem_start[]      asm("_binary_aws_root_ca_pem_start");
static const uint8_t aws_root_ca_pem_end[]        asm("_binary_aws_root_ca_pem_end");

/* Private function prototypes ---------------------------------------------- */
static void m_sys_aws_task(void *params);
static bool m_sys_aws_connect(void);
static void m_sys_aws_disconnect_callback_handler(AWS_IoT_Client *p_client, void *data);

static void m_sys_aws_jobs_next_job_callback(AWS_IoT_Client *p_client,
                                             char *topic_name,
                                             uint16_t topic_name_len,
                                             IoT_Publish_Message_Params *params,
                                             void *p_data);
static void m_sys_aws_provision_success_callback(const char *thing_name);

/* Function definitions ----------------------------------------------------- */
void sys_aws_init(void)
{
  if (!g_sys_aws.initialized)
  {
    sys_aws_provision_init_params_t init_params = { 0 };

    init_params.aws_client_id_len    = strlen(g_nvs_setting_data.mac_device_addr),
    init_params.qr_code_len          = strlen(g_nvs_setting_data.dev.qr_code);
    init_params.provision_success_cb = m_sys_aws_provision_success_callback;

    memcpy(init_params.aws_client_id, g_nvs_setting_data.mac_device_addr, init_params.aws_client_id_len);
    memcpy(init_params.qr_code, g_nvs_setting_data.dev.qr_code, init_params.qr_code_len);

    // Check provisioning status
    if (g_nvs_setting_data.provision_status == AWS_PROVISION_NONE)
    {
      if (FLAG_QRCODE_SET == g_nvs_setting_data.dev.qr_code_flag)
      {
        sys_aws_provision_start(&init_params);
      }
      else if (FLAG_QRCODE_SET_SUCCESS == g_nvs_setting_data.dev.qr_code_flag)
      {
        ESP_LOGI(TAG, "QR code is set successfull");
      }
      else
      {
        ESP_LOGI(TAG, "QR code is not set");
      }
    }
    else
    {
      sys_aws_start();
    }

    g_sys_aws.initialized = true;
  }
}

void sys_aws_start(void)
{
  g_sys_aws.evt_queue = xQueueCreate(20, sizeof(sys_aws_service_t));

  xTaskCreate(m_sys_aws_task,
              "aws_task",
              AWS_TASK_STACK_SIZE,
              NULL,
              AWS_TASK_PRIORITY,
              NULL);
}

/* Private function definitions --------------------------------------------- */
/**
 * @brief         AWS task
 *
 * @param[in]     params    Pointer to params
 *
 * @attention     None
 *
 * @return        None
 */
static void m_sys_aws_task(void *params)
{
  char temp_buf[AWS_IOT_MQTT_TX_BUF_LEN];
  sys_aws_service_t service;

  m_sys_aws_connect();

  // MQTT service
  sys_aws_mqtt_subscribe(AWS_PUB_TOPIC_DOWNNSTREAM);

  // Shadow service
  sys_aws_shadow_init();

  // Jobs service
  sys_aws_jobs_init(&g_sys_aws.client, g_nvs_setting_data.thing_name, m_sys_aws_jobs_next_job_callback);

  while (1)
  {
    aws_iot_mqtt_yield(&g_sys_aws.client, 1000);

    if (aws_iot_mqtt_is_client_connected(&g_sys_aws.client))
    {
      if (xQueueReceive(g_sys_aws.evt_queue, &service, pdMS_TO_TICKS(100)) == pdTRUE)
      {
        ESP_LOGI(TAG, "Queue receive - Service type: %d", service.type);
        ESP_LOGI(TAG, "Queue receive - Service cmd : %d", service.mqtt.cmd);

        if (service.type == SYS_AWS_SHADOW)
        {
          // Handle Shadow
          switch (service.shadow.cmd)
          {
          case SYS_AWS_SHADOW_CMD_GET:
            sys_aws_shadow_get(service.shadow.name);
            break;

          case SYS_AWS_SHADOW_CMD_SET:
            sys_aws_shadow_update(service.shadow.name);
            break;

          default:
            break;
          }
        }
        else
        {
          // Handle MQTT
          switch (service.mqtt.cmd)
          {
          case SYS_AWS_MQTT_CMD_PUB:
            switch (service.mqtt.data.packet_type)
            {
            case AWS_PKT_NOTI:
              aws_build_packet(AWS_PKT_NOTI, &service.mqtt.data.noti_param, 
                                temp_buf, sizeof(temp_buf));
              break;
            
            case AWS_PKT_RESP:
              aws_build_packet(AWS_PKT_RESP, &service.mqtt.data.resp_param, 
                               temp_buf, sizeof(temp_buf));
              break;

            default:
              break;
            }
            
            sys_aws_mqtt_publish(service.mqtt.pub_topic, temp_buf);
            break;

          default:
            break;
          }
        }
      }
    }
  }
}

/**
 * @brief         AWS connect
 *
 * @param[in]     None
 *
 * @attention     None
 *
 * @return
 *  - true:   Connect success
 *  - false:  Connect failed
 */
static bool m_sys_aws_connect(void)
{
  ShadowInitParameters_t    shadow_init_params    = ShadowInitParametersDefault;
  ShadowConnectParameters_t shadow_connect_params = ShadowConnectParametersDefault;

  shadow_init_params.pHost = AWS_HOST_ADDRESS;
  shadow_init_params.port  = AWS_PORT;

  shadow_init_params.pClientCRT        = AWS_OFFICIAL_CERTIFICATE_PATH;
  shadow_init_params.pClientKey        = AWS_OFFICIAL_PRIVATE_KEY_PATH;
  shadow_init_params.pRootCA           = (const char *)aws_root_ca_pem_start;
  shadow_init_params.disconnectHandler = m_sys_aws_disconnect_callback_handler;

  shadow_connect_params.pMyThingName    = (const char *)g_nvs_setting_data.thing_name;
  shadow_connect_params.pMqttClientId   = g_nvs_setting_data.thing_name;
  shadow_connect_params.mqttClientIdLen = (uint16_t) strlen(g_nvs_setting_data.thing_name);

  // AWS shadow init
  ESP_LOGI(TAG, "Shadow init...");
  CHECK(SUCCESS == aws_iot_shadow_init(&g_sys_aws.client, &shadow_init_params), false);

  // Connecting to AWS
  ESP_LOGI(TAG, "Shadow connect...");
  CHECK(SUCCESS == aws_iot_shadow_connect(&g_sys_aws.client, &shadow_connect_params), false);

  return true;
}

/**
 * @brief         AWS disconnect callback handler
 *
 * @param[in]     p_client        Pointer to client
 * @param[in]     data            Pointer to data
 *
 * @attention     None
 *
 * @return        None
 */
static void m_sys_aws_disconnect_callback_handler(AWS_IoT_Client *p_client, void *data)
{
  IoT_Error_t status = FAILURE;
  ESP_LOGW(TAG, "MQTT Disconnect");

  if (NULL == p_client)
    return;

  if (aws_iot_is_autoreconnect_enabled(p_client))
  {
    ESP_LOGI(TAG, "Auto Reconnect is enabled, Reconnecting attempt will start now");
  }
  else
  {
    ESP_LOGI(TAG, "Auto Reconnect not enabled. Starting manual reconnect...");

    status = aws_iot_mqtt_attempt_reconnect(p_client);

    if (NETWORK_RECONNECTED == status)
      ESP_LOGI(TAG, "Manual Reconnect Successful");
    else
      ESP_LOGW(TAG, "Manual Reconnect Failed - %d", status);
  }
}

/**
 * @brief         AWS jobs next job callback
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
static void m_sys_aws_jobs_next_job_callback(AWS_IoT_Client *p_client,
                                             char *topic_name,
                                             uint16_t topic_name_len,
                                             IoT_Publish_Message_Params *params,
                                             void *p_data)
{
  jsmntok_t *tok_execution;
  jsmntok_t *tok_document;
  jsmntok_t *tok;
  char job_id[MAX_SIZE_OF_JOB_ID];
  char job_operation[MAX_SIZE_OF_JOB_OPERATION];

  IOT_UNUSED(p_data);
  IOT_UNUSED(p_client);
  ESP_LOGW(TAG_JOB, "AWS jobs next job callback");
  ESP_LOGW(TAG_JOB, "Topic: %.*s", topic_name_len, topic_name);
  ESP_LOGI(TAG_JOB, "Payload: %.*s", (int)params->payloadLen, (char *)params->payload);

  jsmn_init(&m_json_parser);

  m_token_count = jsmn_parse(&m_json_parser, params->payload, (int)params->payloadLen, m_json_token_struct, MAX_JSON_TOKEN_EXPECTED);
  if (m_token_count < 0)
  {
    ESP_LOGI(TAG_JOB, "Failed to parse JSON: %d", m_token_count);
    return;
  }

  // Assume the top-level element is an object
  if (m_token_count < 1 || m_json_token_struct[0].type != JSMN_OBJECT)
  {
    ESP_LOGI(TAG_JOB, "Top Level is not an object");
    return;
  }

  // Get execution payload
  tok_execution = findToken("execution", params->payload, m_json_token_struct);
  if (tok_execution)
  {
    ESP_LOGI(TAG_JOB, "execution: %.*s", tok_execution->end - tok_execution->start, (char *)params->payload + tok_execution->start);

    // Get jobId payload
    tok = findToken("jobId", params->payload, tok_execution);
    if (tok)
    {
      parseStringValue(job_id, MAX_SIZE_OF_JOB_ID, params->payload, tok);
      ESP_LOGI(TAG_JOB, "jobId: %s", job_id);

      // Get jobDocument payload
      tok_document = findToken("jobDocument", params->payload, tok_execution);
      if (tok_document)
      {
        ESP_LOGI(TAG_JOB, "jobDocument: %.*s", tok_document->end - tok_document->start, (char *)params->payload + tok_document->start);

        // Get operation payload
        tok = findToken("operation", params->payload, tok_document);
        if (tok)
        {
          parseStringValue(job_operation, MAX_SIZE_OF_JOB_OPERATION, params->payload, tok);
          ESP_LOGI(TAG_JOB, "job_operation: %s", job_operation);

          if (0 == strcmp(job_operation, AWS_JOB_OPERATION[AWS_DFU_JOB]))
          {
            // Get url payload
            tok = findToken("url", params->payload, tok_document);
            if (tok)
            {
              // // Get OTA state in nvs
              // switch (g_nvs_setting_data.ota.status)
              // {
              // case OTA_STATE_FAILED:
              //   ESP_LOGW(TAG, "JOB_EXECUTION_FAILED");
              //   sys_aws_jobs_send_update(job_id, JOB_EXECUTION_FAILED);
              //   break;
              
              // case OTA_STATE_SUCCEEDED:
              //   ESP_LOGW(TAG, "JOB_EXECUTION_SUCCEEDED");
              //   sys_aws_jobs_send_update(job_id, JOB_EXECUTION_SUCCEEDED);
              //   break;

              // case OTA_STATE_NONE:
              //   sys_aws_jobs_send_update(job_id, JOB_EXECUTION_IN_PROGRESS);
              //   parseStringValue(g_nvs_setting_data.ota.url, MAX_SIZE_OF_JOB_UPGRADE_URL, params->payload, tok);
                
              //   ESP_LOGW(TAG, "JOB_EXECUTION_IN_PROGRESS");
              //   ESP_LOGI(TAG, "OTA url: %s", g_nvs_setting_data.ota.url);

              //   sys_ota_setup(g_nvs_setting_data.ota.url);
              //   break;
              
              // default:
              //   break;
              // }

              // g_nvs_setting_data.ota.status = OTA_STATE_NONE;
              // SYS_NVS_STORE(ota);
            }
          }
        }
      }
      else
      {
        sys_aws_jobs_send_update(job_id, JOB_EXECUTION_FAILED);
      }
    }
  }
  else
  {
    ESP_LOGI(TAG_JOB,"Execution property not found, nothing to do");
  }
}

/**
 * @brief         AWS provision success callback
 *
 * @param[in]     thing_name          Thing name
 *
 * @attention     None
 *
 * @return        None
 */
static void m_sys_aws_provision_success_callback(const char *thing_name)
{
  // Set status and device ID
  g_nvs_setting_data.provision_status = AWS_PROVISION_DONE;
  g_nvs_setting_data.dev.qr_code_flag = FLAG_QRCODE_SET_SUCCESS;
  strcpy(g_nvs_setting_data.thing_name, thing_name);

  // Save to NVS
  SYS_NVS_STORE(provision_status);
  SYS_NVS_STORE(dev);
  SYS_NVS_STORE(thing_name);

  // Start AWS
  sys_aws_start();
}

/* End of file -------------------------------------------------------------- */