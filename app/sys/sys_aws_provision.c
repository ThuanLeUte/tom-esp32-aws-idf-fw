/**
* @file       sys_aws_provision.c
* @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2022-04-18
* @author     Thuan Le
* @brief      System module to handle Amazon Web Services Provision (AWS)
* @note       None
* @example    None
*/

/* Includes ----------------------------------------------------------------- */
#include <sys/stat.h>
#include "sys_aws_provision.h"
#include "sys_aws_config.h"
#include "sys_nvs.h"

#include "platform_common.h"
#include "aws_iot_config.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "jsmn.h"
#include "aws_iot_json_utils.h"
#include "frozen.h"
#include "esp_spiffs.h"

/* Private enum/structs ----------------------------------------------------- */
/* Private defines ---------------------------------------------------------- */
#define AWS_PROVISION_TASK_STACK_SIZE       (8192 / sizeof(StackType_t))
#define AWS_PROVISION_TASK_PRIORITY         (3)
#define AWS_PROVISION_NUM_SUBSCRIBE_TOPIC   (4)

/* Private Constants -------------------------------------------------------- */
static const char *TAG = "sys/aws_provision";
static const char *AWS_SUBSCRIBE_TOPIC[AWS_PROVISION_NUM_SUBSCRIBE_TOPIC] =
{
  "$aws/provisioning-templates/"AWS_TEMPLATE_NAME"/provision/json/accepted",
  "$aws/provisioning-templates/"AWS_TEMPLATE_NAME"/provision/json/rejected",
  "$aws/certificates/create/json/accepted",
  "$aws/certificates/create/json/rejected"
};

static const char *AWS_CERTIFICATE_CREATE_TOPIC          = "$aws/certificates/create/json";
static const char *AWS_REGISTER_THING_TOPIC              = "$aws/provisioning-templates/"AWS_TEMPLATE_NAME"/provision/json";

static const uint8_t aws_root_ca_pem_start[]      asm("_binary_aws_root_ca_pem_start");
static const uint8_t aws_root_ca_pem_end[]        asm("_binary_aws_root_ca_pem_end");

#if (_CONFIG_ENVIRONMENT_DEV)
static const uint8_t certificate_pem_crt_start[]  asm("_binary_certificate_pem_crt_start");
static const uint8_t certificate_pem_crt_end[]    asm("_binary_certificate_pem_crt_end");
static const uint8_t private_pem_key_start[]      asm("_binary_private_pem_key_start");
static const uint8_t private_pem_key_end[]        asm("_binary_private_pem_key_end");
#endif

/* Private variables -------------------------------------------------------- */
static sys_aws_provision_init_params_t m_provision_params = { 0 };
static char         m_thing_name[50];
static jsmn_parser  m_json_parser;
static jsmntok_t    m_json_token_struct[MAX_JSON_TOKEN_EXPECTED];
static AWS_IoT_Client             m_aws_client;
static IoT_Publish_Message_Params m_params_publish_msg = { 0 };

/* Public variables --------------------------------------------------------- */
/* Private function prototypes ---------------------------------------------- */
static void m_sys_aws_provision_task(void *params);
static bool m_sys_aws_connect(void);
static bool m_sys_aws_enable_monitor(void);
static bool m_sys_aws_get_official_certs(void);
static bool m_sys_aws_register_thing(char *token, int token_len);
static bool m_sys_aws_save_certificates(char *p_path, char *p_data, uint16_t data_len);
static void m_sys_aws_disconnect_callback_handler(AWS_IoT_Client *p_client, void *data);
static void m_sys_aws_subscribe_callback_handler(AWS_IoT_Client             *p_client,
                                                 char                       *topic_name,
                                                 uint16_t                   topic_name_len,
                                                 IoT_Publish_Message_Params *params,
                                                 void                       *p_data);

/* Function definitions ----------------------------------------------------- */
void sys_aws_provision_start(sys_aws_provision_init_params_t *params)
{
  assert(params != NULL);
  assert(params->aws_client_id != NULL);
  assert(params->qr_code != NULL);
  assert(params->provision_success_cb != NULL);
  assert(params->aws_client_id_len > 0);
  assert(params->qr_code_len > 0);

  // Save provision params
  memcpy(&m_provision_params, params, sizeof(m_provision_params));
  
  // Create task to handle provisioning
  xTaskCreate(m_sys_aws_provision_task,
              "aws_provision_task",
              AWS_PROVISION_TASK_STACK_SIZE,
              NULL,
              AWS_PROVISION_TASK_PRIORITY,
              NULL);
}

/* Private function definitions --------------------------------------------- */
/**
 * @brief         AWS provision task
 *
 * @param[in]     params    Pointer to params
 *
 * @attention     None
 *
 * @return        None
 */
static void m_sys_aws_provision_task(void *params)
{
  ESP_LOGI(TAG, "Provision start...");

  // Connect to AWS with provision clain certs
  m_sys_aws_connect();

  // Monitor topics
  m_sys_aws_enable_monitor();

  // Make a publish call to topic to get official certs
  m_sys_aws_get_official_certs();

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
    aws_iot_mqtt_yield(&m_aws_client, 1000);
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
  IoT_Client_Init_Params    mqtt_init_params = iotClientInitParamsDefault;
  IoT_Client_Connect_Params connect_params   = iotClientConnectParamsDefault;

  mqtt_init_params.enableAutoReconnect       = false;
  mqtt_init_params.pHostURL                  = AWS_HOST_ADDRESS;
  mqtt_init_params.port                      = AWS_PORT;

  mqtt_init_params.pRootCALocation           = (const char *)aws_root_ca_pem_start;
  mqtt_init_params.pDeviceCertLocation       = (const char *)certificate_pem_crt_start;
  mqtt_init_params.pDevicePrivateKeyLocation = (const char *)private_pem_key_start;

  mqtt_init_params.mqttCommandTimeout_ms     = 20000;
  mqtt_init_params.tlsHandshakeTimeout_ms    = 5000;
  mqtt_init_params.isSSLHostnameVerify       = true;
  mqtt_init_params.disconnectHandler         = m_sys_aws_disconnect_callback_handler;
  mqtt_init_params.disconnectHandlerData     = NULL;

  connect_params.keepAliveIntervalInSec      = 60;
  connect_params.isCleanSession              = true;
  connect_params.MQTTVersion                 = MQTT_3_1_1;

  connect_params.pClientID                   = (const char *) m_provision_params.aws_client_id;
  connect_params.clientIDLen                 = strlen((const char *) m_provision_params.aws_client_id);
  connect_params.isWillMsgPresent            = false;

  // AWS mqtt init
  ESP_LOGI(TAG, "AWS init...");
  CHECK(SUCCESS == aws_iot_mqtt_init(&m_aws_client, &mqtt_init_params), false);

  // Connecting to AWS
  ESP_LOGI(TAG, "AWS connect...");
  IoT_Error_t err;
  err = aws_iot_mqtt_connect(&m_aws_client, &connect_params);
  ESP_LOGI(TAG, "AWS connect error: %d", err);

  // AWS auto reconnect set
  CHECK(SUCCESS == aws_iot_mqtt_autoreconnect_set_status(&m_aws_client, true), false);

  return true;
}

/**
 * @brief         AWS enable monitor
 *
 * @param[in]     None
 *
 * @attention     None
 *
 * @return
 *  - true:   Enable monitor success
 *  - false:  Enable monitor failed
 */
static bool m_sys_aws_enable_monitor(void)
{
  ESP_LOGI(TAG, "Subscribing to monitor topic...");
  for (int i = 0; i < AWS_PROVISION_NUM_SUBSCRIBE_TOPIC; i++)
  {
    CHECK(SUCCESS == aws_iot_mqtt_subscribe(&m_aws_client, AWS_SUBSCRIBE_TOPIC[i],
                                            strlen(AWS_SUBSCRIBE_TOPIC[i]),
                                            QOS1,
                                            m_sys_aws_subscribe_callback_handler,
                                            NULL), false);
  }

  return true;
}

/**
 * @brief         AWS get official certs
 *
 * @param[in]     None
 *
 * @attention     None
 *
 * @return
 *  - true:   Get official certs success
 *  - false:  Get official certs failed
 */
static bool m_sys_aws_get_official_certs(void)
{
  const char *payload             = "{}";
  m_params_publish_msg.payload    = (void *)payload;
  m_params_publish_msg.payloadLen = strlen(payload);
  m_params_publish_msg.qos        = QOS1;
  m_params_publish_msg.isRetained = 0;

  // Make a publish call to topic to get official certs
  ESP_LOGI(TAG, "Get official certs...");
  CHECK(SUCCESS == aws_iot_mqtt_publish(&m_aws_client,
                                        AWS_CERTIFICATE_CREATE_TOPIC,
                                        strlen(AWS_CERTIFICATE_CREATE_TOPIC),
                                        &m_params_publish_msg), false);

  return true;
}

/**
 * @brief         AWS register thing
 *
 * @param[in]     token           Pointer to token
 * @param[in]     token_len       Token length
 *
 * @attention     None
 *
 * @return
 *  - true:   AWS register thing success
 *  - false:  AWS register thing failed
 */
static bool m_sys_aws_register_thing(char *token, int token_len)
{
  char buf[1000] = "";
  struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));

  // Create json format
  json_printf(&out,
              "{certificateOwnershipToken: %.*Q, parameters: {SerialNumber: %Q}}",
              token_len,
              token,
              (const char *) m_provision_params.qr_code);


  m_params_publish_msg.payload    = (void *)buf;
  m_params_publish_msg.payloadLen = strlen(buf);

  // Register thing / activate certificate
  ESP_LOGI(TAG, "Register thing...: %s", buf);
  CHECK(SUCCESS == aws_iot_mqtt_publish(&m_aws_client,
                                        AWS_REGISTER_THING_TOPIC,
                                        strlen(AWS_REGISTER_THING_TOPIC),
                                        &m_params_publish_msg), false);

  return true;
}

/**
 * @brief         Save certificates to SPIFF
 *
 * @param[in]     p_path        Pointer to save data destination
 * @param[in]     p_data        Pointer data
 * @param[in]     data_len      Data length
 *
 * @attention     None
 *
 * @return
 *    - true:   Save data success
 *    - failed: Save data fail
 */
static bool m_sys_aws_save_certificates(char *p_path, char *p_data, uint16_t data_len)
{
  FILE        *fp = NULL;
  struct stat file_stat;
  uint8_t     index = 1;
  uint16_t    data_start_pos[100];

  unlink(p_path);
  if (0 == stat(p_path, &file_stat))
  {
    fp = fopen(p_path, "r+");
    ESP_LOGI(TAG, "%s: Certificate existed", __FUNCTION__);
  }
  else
  {
    fp = fopen(p_path, "w+");
    ESP_LOGW(TAG, "%s: Certificate not existed", __FUNCTION__);
  }

  if (NULL == fp)
  {
    ESP_LOGE(TAG, "%s: Open file failed", __FUNCTION__);
    return false;
  }

  // Analyze data, remove '\n'
  data_start_pos[0] = 0;
  for (int i = 0; i < data_len; i++)
  {
    if ((p_data[i] == '\\') && (p_data[i+1] == 'n'))
    {
      data_start_pos[index] = i + 2;
      p_data[i] = 0;
      index++;
    }
  }

  // Write data to file system
  for (int i = 0; i < (index - 1); i++)
  {
    fprintf(fp, "%s\n", &p_data[data_start_pos[i]]);
    ESP_LOGW(TAG, "Print data: %s", &p_data[data_start_pos[i]]);
  }

  fclose(fp);

  return true;
}

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
  ESP_LOGW(TAG, "Subscribe callback");
  ESP_LOGI(TAG, "%.*s\t%.*s", topic_name_len, topic_name, (int)params->payloadLen, (char *)params->payload);

  // Callback on rejected topic
  if ((0 == strncmp(topic_name, AWS_SUBSCRIBE_TOPIC[1], topic_name_len)) ||
      (0 == strncmp(topic_name, AWS_SUBSCRIBE_TOPIC[3], topic_name_len)))
  {
    ESP_LOGE(TAG,"Failed provisioning");

    // Disconnect current provision mqtt
    aws_iot_mqtt_disconnect(&m_aws_client);
    vTaskDelete(NULL);
    return;
  }

  // Json parse data that contains certificate data
  jsmn_init(&m_json_parser);
  jsmn_parse(&m_json_parser,
             (char *)params->payload,
             (int)params->payloadLen,
             m_json_token_struct,
             sizeof(m_json_token_struct) / sizeof(m_json_token_struct[0]));

  // A response has been recieved from the service that contains certificate data.
  json_obs = findToken("certificateId", params->payload, m_json_token_struct);
  if (json_obs)
  {
    json_obs = findToken("certificatePem", params->payload, m_json_token_struct);
    if (json_obs)
    {
      m_sys_aws_save_certificates(AWS_OFFICIAL_CERTIFICATE_PATH,
                                  params->payload + json_obs->start,
                                  json_obs->end - json_obs->start);
    }

    json_obs = findToken("privateKey", params->payload, m_json_token_struct);
    if (json_obs)
    {
      m_sys_aws_save_certificates(AWS_OFFICIAL_PRIVATE_KEY_PATH,
                                  params->payload + json_obs->start,
                                  json_obs->end - json_obs->start);
    }

    json_obs = findToken("certificateOwnershipToken", params->payload, m_json_token_struct);
    if (json_obs)
    {
      ESP_LOGI(TAG, "Device Name: %s", m_provision_params.qr_code);

      m_sys_aws_register_thing((char *)params->payload + json_obs->start,
                               json_obs->end - json_obs->start);
    }
  }

  // A response contains acknowledgement that the provisioning template has been actived.
  json_obs = findToken("deviceConfiguration", params->payload, m_json_token_struct);
  if (json_obs)
  {
    ESP_LOGI(TAG, "Activation complete");

    // Disconnect current provision mqtt
    aws_iot_mqtt_disconnect(&m_aws_client);

    // Get thing name
    json_obs = findToken("thingName", params->payload, m_json_token_struct);
    if (json_obs)
    {
      memset(m_thing_name, 0, sizeof(m_thing_name));
      strncpy(m_thing_name, params->payload + json_obs->start, json_obs->end - json_obs->start);
      ESP_LOGI(TAG, "Thing name: %s", m_thing_name);

      m_provision_params.provision_success_cb(m_thing_name);
    }

    vTaskDelete(NULL);
  }
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
  ESP_LOGW(TAG, "MQTT Disconnect");
  IoT_Error_t status = FAILURE;

  if (NULL == p_client)
    return;

  if (aws_iot_is_autoreconnect_enabled(p_client))
  {
    ESP_LOGI(TAG, "Auto Reconnect is enabled, Reconnecting attempt will start now");
  }
  else
  {
    ESP_LOGW(TAG, "Auto Reconnect not enabled. Starting manual reconnect...");

    status = aws_iot_mqtt_attempt_reconnect(p_client);
    if (NETWORK_RECONNECTED == status)
      ESP_LOGI(TAG, "Manual Reconnect Successful");
    else
      ESP_LOGW(TAG, "Manual Reconnect Failed - %d", status);
  }
}

/* End of file -------------------------------------------------------------- */
