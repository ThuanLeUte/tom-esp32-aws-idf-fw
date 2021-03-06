/**
 * @file       sys.c
 * @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
 * @license    This project is released under the Hydratech License.
 * @version    1.0.0
 * @date       2022-01-13
 * @author     Thuan Le
 * @brief      System file
 * @note       None
 * @example    None
 */

/* Includes ----------------------------------------------------------- */
#include "sys.h"
#include "sys_variant.h"
#include "sys_wifi.h"
#include "sys_aws_provision.h"
#include "sys_nvs.h"
#include "sys_devcfg.h"
#include "bsp.h"
#include "sys_aws.h"
#include "sys_ota.h"
#include "sys_http_server.h"
#include "bsp_error.h"

/* Private defines ---------------------------------------------------- */
static const char *TAG = "sys";

#define SYS_EVENT_TBL_IT(evt)    {evt, #evt}

/* Private enumerate/structure ---------------------------------------- */
typedef struct
{
  system_event_id_t event;
  const char *msg;
}
sys_event_msg_t;

/* Private macros ----------------------------------------------------- */
/* Public variables --------------------------------------------------- */
sys_device_t g_device;
EventGroupHandle_t g_sys_evt_group;

/* Private variables -------------------------------------------------- */
static const sys_event_msg_t SYS_EVENT_MSG_TABLE[] = 
{
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_WIFI_READY),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_SCAN_DONE),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_STA_START),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_STA_STOP),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_STA_CONNECTED ),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_STA_DISCONNECTED),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_STA_AUTHMODE_CHANGE),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_STA_GOT_IP),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_STA_LOST_IP),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_STA_BSS_RSSI_LOW),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_STA_WPS_ER_SUCCESS),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_STA_WPS_ER_FAILED),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_STA_WPS_ER_TIMEOUT),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_STA_WPS_ER_PIN),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_STA_WPS_ER_PBC_OVERLAP),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_AP_START),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_AP_STOP),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_AP_STACONNECTED),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_AP_STADISCONNECTED),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_AP_STAIPASSIGNED),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_AP_PROBEREQRECVED),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_ACTION_TX_STATUS),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_ROC_DONE),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_STA_BEACON_TIMEOUT),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_FTM_REPORT),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_GOT_IP6),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_ETH_START),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_ETH_STOP ),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_ETH_CONNECTED),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_ETH_DISCONNECTED),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_ETH_GOT_IP),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_ETH_LOST_IP),
  SYS_EVENT_TBL_IT(SYSTEM_EVENT_MAX)
};

/* Private Constants -------------------------------------------------- */
/* Private function prototypes ---------------------------------------- */
static void m_sys_evt_group_init(void);
static bool m_sys_self_test(void);
static void m_sys_firmware_rollback_check(void);

/* Function definitions ----------------------------------------------- */
void sys_boot(void)
{
  sys_nvs_init();
  bsp_spiffs_init();
  m_sys_evt_group_init();
  bsp_error_init();

  // WiFi Setup ---------------------------------- {
  sys_wifi_init();

  // Check the device is provision or not
  if (FLAG_QRCODE_SET_SUCCESS == g_nvs_setting_data.dev.qr_code_flag)
  {
    // Check perious WiFi mode.
    if (g_nvs_setting_data.wifi.mode == SYS_WIFI_MODE_STA)
    {
__LBL_WEBPAGE_SETUP_:
#if (__CONFIG_SOFT_AP_MODE)
      // Enter WiFi setup by Web server.
      sys_wifi_softap_init();
      sys_http_server_start();

      // Save current WiFi mode.
      g_nvs_setting_data.wifi.mode = SYS_WIFI_MODE_AP;
      SYS_NVS_STORE(wifi);

      FSM_UPDATE_STATE(SYS_STATE_NW_SETUP);
#else
      goto __LBL_WIFI_SETUP_;
#endif // __CONFIG_SOFT_AP_MODE
    }
    else
    {
      // Check WiFi setup in the STA mode
      if ((strlen((char *)g_nvs_setting_data.wifi.pwd) != 0) && (strlen((char *)g_nvs_setting_data.wifi.uiid) != 0))
      {
#if (!__CONFIG_SOFT_AP_MODE)
__LBL_WIFI_SETUP_:
#endif // __CONFIG_SOFT_AP_MODE

        // Save current WiFi mode.
        g_nvs_setting_data.wifi.mode = SYS_WIFI_MODE_STA;
        SYS_NVS_STORE(wifi);

        sys_wifi_sta_init();
        sys_wifi_config_set(g_nvs_setting_data.wifi.uiid, g_nvs_setting_data.wifi.pwd);
        sys_wifi_sta_start();

        // Check OTA enable or not
        if (g_nvs_setting_data.ota.enable)
          sys_ota_start();
        else
          FSM_UPDATE_STATE(SYS_STATE_READY);
      }
      else // WiFi is not setup --> Enter Webpage setup again
      {
        goto __LBL_WEBPAGE_SETUP_;
      }
    }
  }
  else // Setup WiFi by BluFi at the Factory
  {
    sys_wifi_sta_init();

    if (!sys_wifi_is_configured())
    {
      sys_devcfg_init();
      FSM_UPDATE_STATE(SYS_STATE_NW_SETUP);
    }
    else
    {
      // Save current WiFi mode.
      g_nvs_setting_data.wifi.mode = SYS_WIFI_MODE_AP;
      SYS_NVS_STORE(wifi);

      sys_wifi_sta_start();
      FSM_UPDATE_STATE(SYS_STATE_READY);
    }
  }
  // ------------------------------------------ }
}

void sys_run(void)
{
  switch (g_device.sys_state)
  {
  case SYS_STATE_POWER_ON:
  {
    bsp_delay_ms(1000);
    break;
  }

  case SYS_STATE_NW_SETUP:
  {
    bsp_delay_ms(1000);
    break;
  }
  
  case SYS_STATE_READY:
  {
    bsp_delay_ms(50000);

    uint32_t alarm_code = 1111;
    sys_aws_mqtt_send_noti(AWS_NOTI_ALARM, &alarm_code);
    bsp_delay_ms(5000);

    aws_noti_dev_data_t device_data;
    sprintf(device_data.serial_number, g_nvs_setting_data.dev.qr_code ); 
    
    /* active devices 
    ESP32C3_B2A6  -  serial # "141A14191A18"
    ESP32C3_BBC6  -  serial # "1812454ABC"
    */
    
    device_data.weight_scale = 1340;
    device_data.temp         = 101;
    device_data.battery      = 99;
    device_data.alarm_code   = 11;

    device_data.longitude = -84.3067;
    device_data.lattitude = 34.1351;
    
    sys_aws_mqtt_send_noti(AWS_NOTI_DEVICE_DATA, &device_data);

    break;
  }

  default:
    break;
  }
}

void sys_event_group_set(const EventBits_t bit_to_set)
{
  if (g_sys_evt_group != NULL)
    xEventGroupSetBits(g_sys_evt_group, bit_to_set);
}

void sys_event_group_clear(const EventBits_t bit_to_clear)
{
  if (g_sys_evt_group != NULL)
    xEventGroupClearBits(g_sys_evt_group, bit_to_clear);
}

const char *sys_event_id_to_name(system_event_id_t evt)
{
  for (uint16_t i = 0; i < sizeof(SYS_EVENT_MSG_TABLE) / sizeof(SYS_EVENT_MSG_TABLE[0]); ++i)
  {
    if (SYS_EVENT_MSG_TABLE[i].event == evt)
    {
      return SYS_EVENT_MSG_TABLE[i].msg;
    }
  }

  return "EVENT NONE";
}




/* Private function --------------------------------------------------------- */
/**
 * @brief System event group init
 */
static void m_sys_evt_group_init(void)
{
  g_sys_evt_group = xEventGroupCreate();
}

/**
 * @brief         Firmware rollback check
 * 
 * @param[in]     None
 * 
 * @attention     None
 * 
 * @return        None
 */
static void m_sys_firmware_rollback_check(void)
{
  esp_ota_img_states_t ota_state;
  const esp_partition_t *running = esp_ota_get_running_partition();

  if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK)
  {
    if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
    {
      if (m_sys_self_test())
      {
        ESP_LOGI(TAG, "Self-test completed successfully! Continuing execution ...");
        esp_ota_mark_app_valid_cancel_rollback();
      }
      else
      {
        ESP_LOGE(TAG, "Self-test failed! Start rollback to the previous version ...");
        esp_ota_mark_app_invalid_rollback_and_reboot();
      }
    }
  }
}

/**
 * @brief         Firmware rollback check
 * 
 * @param[in]     None
 * 
 * @attention     None
 * 
 * @return        true: Self-test successful
 *                false: Self-test failed
 */
static bool m_sys_self_test(void)
{
  return true;
}

/* End of file -------------------------------------------------------- */