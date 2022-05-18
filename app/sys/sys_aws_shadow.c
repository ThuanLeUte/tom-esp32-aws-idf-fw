/**
* @file       sys_aws_shadow.c
* @copyright  Copyright (C) 2021 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2021-08-17
* @author     Thuan Le
* @brief      System module to handle Amazon Web Services Shadow (AWS)
* @note       None
* @example    None
*/

/* Includes ----------------------------------------------------------------- */
#include "sys_aws_shadow.h"
#include "sys_aws_config.h"
#include "sys_nvs.h"
#include "sys_aws.h"
#include "aws_parser.h"

#include "platform_common.h"
#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_json_utils.h"

#include "frozen.h"
#include "jsmn.h"

/* Private enum/structs ----------------------------------------------------- */
typedef struct
{
  const char *name;
}
sys_shadow_t;

/* Private defines ---------------------------------------------------------- */
#define SHADOW_INFO(_type, _name)[_type] {.name = _name}

#define AWS_MAX_JSON_BUFF         (1000)

/* Private Constants -------------------------------------------------------- */
static const char *TAG = "sys/aws_shadow";

static const sys_shadow_t SHADOW_TABLE[] =
{
  //          +==================================+=====================+
  //          | Shadow                           | Name                |
  //          +----------------------------------+---------------------+
     SHADOW_INFO(SYS_SHADOW_FIRMWARE_ID          , "firmware_id"       )
    ,SHADOW_INFO(SYS_SHADOW_SCALE_TARE           , "scale_tare"        )
  //          +==================================+=====================+
};

/* Private variables -------------------------------------------------------- */
static char m_json_buffer[AWS_MAX_JSON_BUFF];
static size_t m_size_json_buffer = sizeof(m_json_buffer) / sizeof(m_json_buffer[0]);

static jsonStruct_t m_json_struct[SYS_SHADOW_MAX];

/* Public variables --------------------------------------------------- */
/* Private function prototypes ------------------------------- */
static void m_shadow_json_init(void);
static void m_shadow_create_json_format(char *json_buffer, sys_aws_shadow_name_t name);

static void m_parse_shadow_get_payload(sys_aws_shadow_name_t name, const char *buf, uint16_t buf_len);

static void m_shadow_scale_tare_callback(const char *p_json_string, uint32_t json_data_len, jsonStruct_t *p_context);
static void m_shadow_schedule_timeline_callback(const char *p_json_string, uint32_t json_data_len, jsonStruct_t *p_context);
static void m_shadow_baseline_callback(const char *p_json_string, uint32_t json_data_len, jsonStruct_t *p_context);

static void m_shadow_get_callback(const char          *p_thing_name,
                                  const char          *p_shadow_name,
                                  ShadowActions_t     action,
                                  Shadow_Ack_Status_t status,
                                  const char          *p_received_json,
                                  void                *p_context_data);
static void m_shadow_update_status_callback(const char          *p_thing_name,
                                            const char          *p_shadow_name,
                                            ShadowActions_t     action,
                                            Shadow_Ack_Status_t status,
                                            const char          *p_received_json,
                                            void                *p_context_data);

/* Function definitions ----------------------------------------------------- */
bool sys_aws_shadow_init(void)
{
  // AWS shadow json init. Register callback and key
  m_shadow_json_init();

  // AWS register delta
  CHECK(SUCCESS == aws_iot_shadow_register_delta(&g_sys_aws.client, SHADOW_TABLE[SYS_SHADOW_SCALE_TARE].name, &m_json_struct[SYS_SHADOW_SCALE_TARE]), false);

  // NOTE: Device will gets status of shadow on AWS first then 
  //       will update new status on AWS even device call shadow update
  //       The value on the AWS is the final value for device

  // Get data from AWS
  sys_aws_shadow_trigger_command(SYS_AWS_SHADOW_CMD_GET, SYS_SHADOW_SCALE_TARE);     // Get data first time

  // Send Firmware ID
  sys_aws_shadow_trigger_command(SYS_AWS_SHADOW_CMD_SET, SYS_SHADOW_FIRMWARE_ID);
  sys_aws_shadow_trigger_command(SYS_AWS_SHADOW_CMD_SET, SYS_SHADOW_SCALE_TARE);

  return true;
}

void sys_aws_shadow_trigger_command(sys_aws_shadow_cmd_t cmd, sys_aws_shadow_name_t name)
{
  if (!aws_iot_mqtt_is_client_connected(&g_sys_aws.client))
  {
    ESP_LOGW(TAG, "Aws shadow is not connected !. Update event update to queue");
  }

  g_sys_aws.service.type        = SYS_AWS_SHADOW;
  g_sys_aws.service.shadow.cmd  = cmd;
  g_sys_aws.service.shadow.name = name;
  xQueueSend(g_sys_aws.evt_queue, (void *)&g_sys_aws.service, pdMS_TO_TICKS(100));
}

bool sys_aws_shadow_update(sys_aws_shadow_name_t name)
{
  ESP_LOGI(TAG, "Shadow update...");

  CHECK(SUCCESS == aws_iot_shadow_init_json_document(m_json_buffer, m_size_json_buffer), false);

  CHECK(SUCCESS == aws_iot_shadow_add_desired(m_json_buffer, m_size_json_buffer), false);

  m_shadow_create_json_format(m_json_buffer, name);

  CHECK(SUCCESS == aws_iot_shadow_add_reported(m_json_buffer, m_size_json_buffer), false);

  m_shadow_create_json_format(m_json_buffer, name);

  CHECK(SUCCESS == aws_iot_finalize_json_document(m_json_buffer, m_size_json_buffer), false);

  ESP_LOGI(TAG, "Json buffer: %s", m_json_buffer);

  IoT_Error_t rc  = aws_iot_shadow_update(&g_sys_aws.client, (const char *)g_nvs_setting_data.thing_name,
                                          SHADOW_TABLE[name].name, m_json_buffer,
                                          m_shadow_update_status_callback,
                                          NULL, 4, true);

  ESP_LOGI(TAG, "Shadow update error code: %d", rc);

  if (SUCCESS != rc)
    return false;

  CHECK(SUCCESS == aws_iot_shadow_yield(&g_sys_aws.client, 200), false);

  return true;
}

bool sys_aws_shadow_get(sys_aws_shadow_name_t name)
{
  IoT_Error_t rc = aws_iot_shadow_get(&g_sys_aws.client, (const char *)g_nvs_setting_data.thing_name,
                                      SHADOW_TABLE[name].name, m_shadow_get_callback,
                                      NULL, 4, true);

  ESP_LOGI(TAG, "Shadow get error code: %d", rc);

  if (SUCCESS != rc)
    return false;

  return true;
}

/* Private function definitions --------------------------------------------- */
/**
 * @brief         AWS shadow create json format
 *
 * @param[in]     json_buffer     Pointer to json buffer
 * @param[in]     name            Shadow name
 *
 * @attention     None
 *
 * @return        None
 */
static void m_shadow_create_json_format(char *json_buffer, sys_aws_shadow_name_t name)
{
  char buf[AWS_MAX_JSON_BUFF];
  struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));

  // Create json format
  switch (name)
  {
  case SYS_SHADOW_FIRMWARE_ID:
  {
    json_printf(&out, "{data:{fw: %Q}}", DEVICE_FIRMWARE_VERSION);
    break;
  }

  case SYS_SHADOW_SCALE_TARE:
  {
    json_printf(&out, "{data:{scare_tare: %d}}",  g_nvs_setting_data.scale_tare);
    break;
  }

  default:
    break;
  }

  snprintf(json_buffer + strlen(json_buffer), AWS_MAX_JSON_BUFF, buf);
}

/**
 * @brief         AWS shadow json init. Register callback and key
 *
 * @param[in]     None
 *
 * @attention     None
 *
 * @return        None
 */
static void m_shadow_json_init(void)
{
  m_json_struct[SYS_SHADOW_SCALE_TARE].cb   = m_shadow_scale_tare_callback;
  m_json_struct[SYS_SHADOW_SCALE_TARE].pKey = SHADOW_TABLE[SYS_SHADOW_SCALE_TARE].name;
}

/**
 * @brief         Parse payload shadow get
 *
 * @param[in]     buf         Pointer to payload buffer
 * @param[in]     buf_len     Payload buffer length
 *
 * @attention     None
 *
 * @one
*/
static void m_parse_shadow_get_payload(sys_aws_shadow_name_t name, const char *buf, uint16_t buf_len)
{
  // NOTE: Remove "{desired:" in the data buffer
  uint16_t offset = sizeof("{desired:") + 1;

  switch (name)
  {
  case SYS_SHADOW_SCALE_TARE:
    m_shadow_scale_tare_callback((const char *)&buf[offset], buf_len, NULL);
    break;

  default:
    break;
  }
}

/**
 * @brief         AWS shadow get status callback
 *
 * @param[in]     p_thing_name      Pointer to thing name
 * @param[in]     p_shadow_name     Pointer to shadow name
 * @param[in]     action            Action
 * @param[in]     stattus           Status
 * @param[in]     p_received_json   Pointer to received json
 * @param[in]     p_context_data    Pointer to context data
 *
 * @attention     None
 *
 * @return        None
*/
static void m_shadow_get_callback(const char          *p_thing_name,
                                  const char          *p_shadow_name,
                                  ShadowActions_t     action,
                                  Shadow_Ack_Status_t status,
                                  const char          *p_received_json,
                                  void                *p_context_data)
{
  IOT_UNUSED(p_thing_name);
  IOT_UNUSED(p_shadow_name);
  IOT_UNUSED(action);
  IOT_UNUSED(p_context_data);

  jsmntok_t   *json_obs;
  jsmn_parser json_parser;
  jsmntok_t   json_token_struct[MAX_JSON_TOKEN_EXPECTED];

  // Json parse data that contains desired and reported value
  jsmn_init(&json_parser);
  jsmn_parse(&json_parser,
             (const char *)p_received_json,
             (int)strlen(p_received_json),
             json_token_struct,
             sizeof(json_token_struct) / sizeof(json_token_struct[0]));

  json_obs = findToken("state", p_received_json, json_token_struct);
  if (json_obs)
  {
    ESP_LOGI(TAG, "Shadow get callback");
    printf("Payload: %.*s\n", json_obs->end - json_obs->start, p_received_json + json_obs->start);

    for (uint16_t i = 0; i < SYS_SHADOW_MAX; i++)
    {
      if (0 == strcmp(p_shadow_name, SHADOW_TABLE[i].name))
      {
        m_parse_shadow_get_payload(i, p_received_json + json_obs->start, json_obs->end - json_obs->start);
      }
    }
  }
}

/**
 * @brief         AWS shadow callback
 *
 * @param[in]     p_json_string     Pointer to json string
 * @param[in]     json_data_len     Json data length
 * @param[in]     p_received_json   Pointer to received json
 * @param[in]     p_context         Pointer to context data
 *
 * @attention     None
 *
 * @return        None
*/
static void m_shadow_scale_tare_callback(const char *p_json_string, uint32_t json_data_len, jsonStruct_t *p_context)
{
  uint16_t scale_tare;

  if (aws_parse_shadow_packet(SYS_SHADOW_SCALE_TARE, p_json_string, json_data_len, &scale_tare))
  {
    g_nvs_setting_data.scale_tare = scale_tare;
    ESP_LOGI(TAG, "Scare tare: %d", g_nvs_setting_data.scale_tare);
    SYS_NVS_STORE(scale_tare);

    sys_aws_shadow_trigger_command(SYS_AWS_SHADOW_CMD_SET, SYS_SHADOW_SCALE_TARE);
  }
  else
  {
    ESP_LOGW(TAG, "Parsing baseline faild");
  }
}

/**
 * @brief         AWS shadow update status callback
 *
 * @param[in]     p_thing_name      Pointer to thing name
 * @param[in]     p_shadow_name     Pointer to shadow name
 * @param[in]     action            Action
 * @param[in]     stattus           Status
 * @param[in]     p_received_json   Pointer to received json
 * @param[in]     p_context_data    Pointer to context data
 *
 * @attention     None
 *
 * @return        None
*/
static void m_shadow_update_status_callback(const char          *p_thing_name,
                                            const char          *p_shadow_name,
                                            ShadowActions_t     action,
                                            Shadow_Ack_Status_t status,
                                            const char          *p_received_json,
                                            void                *p_context_data)
{
  IOT_UNUSED(p_thing_name);
  IOT_UNUSED(p_shadow_name);
  IOT_UNUSED(action);
  IOT_UNUSED(p_received_json);
  IOT_UNUSED(p_context_data);

  switch (status)
  {
  case SHADOW_ACK_TIMEOUT:
  {
    ESP_LOGE(TAG, "Update timed out");
    ESP_LOGE(TAG, "Shadow name = %s", p_shadow_name);
    break;
  }
  case SHADOW_ACK_REJECTED:
  {
    ESP_LOGE(TAG, "Update rejected");
    break;
  }
  case SHADOW_ACK_ACCEPTED:
  {
    ESP_LOGI(TAG, "Update accepted");
    break;
  }
  default:
    break;
  }
}

/* End of file -------------------------------------------------------------- */
