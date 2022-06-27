/**
 * @file       sys_io.c
 * @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
 * @license    This project is released under the Hydratech License.
 * @version    1.0.0
 * @date       2022-02-18
 * @author     Thuan Le
 * @brief      System module to handle IO
 * @note       None
 * @example    None
 */

/* Includes ----------------------------------------------------------------- */
#include "sys_wifi.h"
#include "sys_aws.h"
#include "sys_devcfg.h"
#include "sys_time.h"
#include "bsp_timer.h"
#include "frozen.h"
#include "sys.h"

/* Private enum/structs ----------------------------------------------------- */
static struct
{
  wifi_config_t config;
  bool is_connected;
  bool is_softap_mode;
  bool is_scan_done;

} m_wifi =
{
  .is_connected = false,
  .is_scan_done = false,
};

static auto_timer_t m_wifi_softap_atm;
static uint8_t station_cnt = 0;

/* Private defines ---------------------------------------------------------- */
static const char *TAG = "sys_wifi";

#define ESP_WIFI_CHANNEL                (1)
#define MAX_STA_CONN                    (10)
#define SOFT_ACCESS_POINT_WAIT_TIME     (30 * ONE_SECOND) 
#define BLE_ACCESS_POINT_WAIT_TIME      (30 * ONE_SECOND) 

/* Private variables -------------------------------------------------------- */
/* Private function prototypes ---------------------------------------------- */
static esp_err_t m_sys_wifi_event_handler(void *ctx, system_event_t *event);
static void m_wifi_softap_device_handler(void *arg, esp_event_base_t event_base,
                                         int32_t event_id, void *event_data);
static void m_sys_wifi_softap_expired_callback(void *para);

/* Function definitions ----------------------------------------------------- */
void sys_wifi_init(void)
{
  esp_err_t err;

  // Init TCP/IP stack
  err = esp_netif_init();
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Esp netif init error: %s", esp_err_to_name(err));
    goto _LBL_END;
  }

  err = esp_event_loop_create_default();
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Esp event loop create default error: %s", esp_err_to_name(err));
    goto _LBL_END;
  }

  // Wifi ssid manager create
  g_ssid_manager = wifi_ssid_manager_create(WIFI_MAX_STATION_NUM);

  return;

_LBL_END:
  ESP_ERROR_CHECK(err);
}

void sys_wifi_sta_init(void)
{
  esp_err_t err;

  m_wifi.is_softap_mode = false;

  esp_netif_create_default_wifi_sta();

  // Init ESP WiFi
  wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();

  err = esp_wifi_init(&wifi_init_cfg);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Esp wifi init error: %s", esp_err_to_name(err));
    ESP_ERROR_CHECK(err);
  }
}

void sys_wifi_softap_init(void)
{
  esp_err_t err;

  m_wifi.is_softap_mode = true;

  esp_netif_create_default_wifi_ap();

  // Init ESP WiFi
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

  err = esp_wifi_init(&cfg);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Esp wifi init error: %s", esp_err_to_name(err));
    goto _LBL_END;
  }

  err = esp_event_handler_instance_register(WIFI_EVENT,
                                            ESP_EVENT_ANY_ID,
                                            &m_wifi_softap_device_handler,
                                            NULL,
                                            NULL);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Esp event handler instance register error: %s", esp_err_to_name(err));
    goto _LBL_END;
  }

  wifi_config_t wifi_config = {
      .ap = {
          .channel        = ESP_WIFI_CHANNEL,
          .max_connection = MAX_STA_CONN,
          .authmode       = WIFI_AUTH_WPA_WPA2_PSK},
  };

  if (g_nvs_setting_data.soft_ap.is_change == false)
    sprintf(g_nvs_setting_data.soft_ap.ssid, "%s_%s", ESP_WIFI_SSID_DEFAULT_AP, g_nvs_setting_data.dev.qr_code);

  memcpy(wifi_config.ap.ssid, g_nvs_setting_data.soft_ap.ssid, strlen(g_nvs_setting_data.soft_ap.ssid));
  memcpy(wifi_config.ap.password, g_nvs_setting_data.soft_ap.pwd, strlen(g_nvs_setting_data.soft_ap.pwd));
  wifi_config.ap.ssid_len = strlen(g_nvs_setting_data.soft_ap.ssid);

  ESP_LOGI(TAG, "SoftAP will be broadcast as: %s", g_nvs_setting_data.soft_ap.ssid);
  ESP_LOGI(TAG, "SSID: %s .Password: %s",  wifi_config.ap.ssid, wifi_config.ap.password);

  err = esp_wifi_set_mode(WIFI_MODE_APSTA);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Esp wifi set mode error: %s", esp_err_to_name(err));
    goto _LBL_END;
  }

  err = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Esp wifi set config error: %s", esp_err_to_name(err));
    goto _LBL_END;
  }

  err = esp_wifi_start();
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Esp wifi start error: %s", esp_err_to_name(err));
    goto _LBL_END;
  }

  // Start time for wait time
  bsp_tmr_auto_init(&m_wifi_softap_atm, m_sys_wifi_softap_expired_callback);
  bsp_tmr_auto_start(&m_wifi_softap_atm, SOFT_ACCESS_POINT_WAIT_TIME);

  ESP_LOGI(TAG, "WiFi init finished. SSID:%s Password:%s",  wifi_config.ap.ssid, wifi_config.ap.password);
  ESP_LOGI(TAG, "Device IP: 192.168.4.1");

  return;

_LBL_END:
  ESP_ERROR_CHECK(err);
}

void sys_wifi_sta_start(void)
{
  esp_err_t err;

  err = esp_event_loop_init(m_sys_wifi_event_handler, NULL);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Esp wifi loop init error: %s", esp_err_to_name(err));
    goto _LBL_END;
  }

  err = esp_wifi_set_mode(WIFI_MODE_STA);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Esp wifi loop set mode error: %s", esp_err_to_name(err));
    goto _LBL_END;
  }

  err = esp_wifi_start();
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Esp wifi start error: %s", esp_err_to_name(err));
    goto _LBL_END;
  }

  return;

_LBL_END:
  ESP_ERROR_CHECK(err);
}

bool sys_wifi_is_connected(void)
{
  return m_wifi.is_connected;
}

void sys_wifi_set_connection_status(bool status)
{
  m_wifi.is_connected = status;
}

void sys_wifi_erase_config(void)
{
  esp_err_t err;

  // Delete network configs
  err = esp_wifi_get_config(WIFI_IF_STA, &m_wifi.config);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Esp wifi set config error: %s", esp_err_to_name(err));
    goto _LBL_END;
  }
  
  memset(m_wifi.config.sta.ssid, 0, sizeof(m_wifi.config.sta.ssid));
  memset(m_wifi.config.sta.password, 0, sizeof(m_wifi.config.sta.password));

  err = esp_wifi_set_config(WIFI_IF_STA, &m_wifi.config);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Esp wifi set config error: %s", esp_err_to_name(err));
    goto _LBL_END;
  }

  return;

_LBL_END:
  ESP_ERROR_CHECK(err);
}

void sys_wifi_config_set(const char *ssid, const char *pwd)
{
    strcpy((char *)m_wifi.config.sta.ssid, ssid);
    strcpy((char *)m_wifi.config.sta.password, pwd);

    ESP_LOGI(TAG, "SSID init: %s", m_wifi.config.sta.ssid);
    ESP_LOGI(TAG, "Password init: %s", m_wifi.config.sta.password);

    esp_wifi_set_config(WIFI_IF_STA, &m_wifi.config);
}

bool sys_wifi_is_configured(void)
{
  esp_wifi_get_config(WIFI_IF_STA, &m_wifi.config); // Get stored WiFi configs

  if (strlen((char *)m_wifi.config.sta.ssid) == 0)
    return false;
    
  return true;
}

void sys_wifi_scan_start(void)
{
  esp_err_t err;

  wifi_scan_config_t scan_cfg = 
    {
      .ssid        = NULL,
      .bssid       = NULL,
      .channel     = 0,
      .show_hidden = true,
      .scan_type   = WIFI_SCAN_TYPE_ACTIVE
    };

  err = esp_wifi_scan_start(&scan_cfg, true);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Esp wifi scan error: %s", esp_err_to_name(err));
    ESP_ERROR_CHECK(err);
  }
}

void sys_wifi_get_scan_wifi_list(char *buf, uint16_t size)
{
  struct json_out out = JSON_OUT_BUF(buf, size);
  uint16_t ap_cnt = 0;

  // Get number of APs found
  esp_wifi_scan_get_ap_num(&ap_cnt);
  if (ap_cnt == 0)
  {
    ESP_LOGW(TAG, "No AP found");
    return;
  }

  // Get AP list
  wifi_ap_record_t *ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ap_cnt);
  ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_cnt, ap_list));

  for (int i = 0; i < ap_cnt; ++i)
  {
    ESP_LOGW(TAG, "WiFi SSID: %s", ap_list[i].ssid);
  }

  ESP_LOGW(TAG, "AP cnt: %d", ap_cnt);

  // json_printf(&out, "{type: wifi_list, cnt: %d}", ap_cnt);

  json_printf(&out, "{type: wifi_list, cnt: %d, list: [", ap_cnt);

  for (uint16_t i = 0; i < ap_cnt; i++)
  {
    json_printf(&out, "%Q", ap_list[i].ssid);

    if (i != (ap_cnt - 1))
      json_printf(&out, ",");
  }

  json_printf(&out, "]}");

  ESP_LOGW(TAG, "WiFi list: %s", buf);

  // Cleanup
  esp_wifi_scan_stop();
  free(ap_list);
}

bool sys_wifi_is_scan_done(void)
{
  return m_wifi.is_scan_done;
}

void sys_wifi_set_wifi_scan_status(bool done)
{
  m_wifi.is_scan_done = done;
}

/* Private function --------------------------------------------------------- */
/**
 * @brief         Wifi connect
 *
 * @param[in]     None
 *
 * @attention     None
 *
 * @return
 *    - true:  Get best config success
 *    - false: Get best config failed
 */
bool sys_wifi_connect(const char *ssid, const char *password)
{
  wifi_config_t config = {0};
  esp_err_t err;

  if (m_wifi.is_softap_mode == false)
  {
    wifi_ssid_manager_list_show(g_ssid_manager);

    err = wifi_ssid_manager_get_best_config(g_ssid_manager, &config);

    if (ESP_OK == err)
    {
__LBL_WIFI_CONNECT__:
      ESP_LOGW(TAG, "Selected SSID: %s (%s)", config.sta.ssid, config.sta.password);
      esp_wifi_set_config(WIFI_IF_STA, &config);
      esp_wifi_connect();
      return true;
    }
    else
    {
      ESP_LOGE(TAG, "Esp wifi manager get bet config error: %s", esp_err_to_name(err));
      ESP_LOGW(TAG, "Can not get the WiFi station in the WiFi save list");
      esp_wifi_connect();
      return false;
    }
  }
  else
  {
    strcpy((char *)config.sta.ssid, (char *)ssid);
    strcpy((char *)config.sta.password, (char *)password);
    goto __LBL_WIFI_CONNECT__;
  }
}

/**
 * @brief         Wifi event handler
 *
 * @param[in]     event           Pointer to event
 *
 * @attention     None
 *
 * @return        esp_err_t
 */
static esp_err_t m_sys_wifi_event_handler(void *ctx, system_event_t *event)
{
  ESP_LOGI(TAG, "Event: %s", sys_event_id_to_name(event->event_id));

  switch (event->event_id)
  {
  case SYSTEM_EVENT_STA_START:
  {
    sys_wifi_connect(NULL, NULL);
    break;
  }
  case SYSTEM_EVENT_STA_GOT_IP:
  {
    m_wifi.is_connected = true;
    ESP_LOGI(TAG, "Connected!");
    sys_time_init();

    if (g_sys_aws.initialized)
    {
      sys_event_group_set(SYS_AWS_RECONNECT_EVT);
    }

    if (!g_nvs_setting_data.ota.enable)
      sys_aws_init();
    break;
  }
  case SYSTEM_EVENT_STA_DISCONNECTED:
  {
    ESP_LOGW(TAG, "Disconnected!");
    m_wifi.is_connected = false;

    sys_wifi_connect(NULL, NULL); // WORKAROUND: as ESP32 WiFi libs don't currently auto-reassociate
    break;
  }

  default:
    break;
  }

  return ESP_OK;
}

static void m_wifi_softap_device_handler(void *arg, esp_event_base_t event_base,
                                         int32_t event_id, void *event_data)
{
  wifi_event_ap_staconnected_t    *event_connected    = (wifi_event_ap_staconnected_t *)event_data;
  wifi_event_ap_stadisconnected_t *event_disconnected = (wifi_event_ap_stadisconnected_t *)event_data;

  ESP_LOGI(TAG, "Event: %s", sys_event_id_to_name(event_id));

  switch (event_id)
  {
  case SYSTEM_EVENT_SCAN_DONE:
    ESP_LOGI(TAG, "Scan done");
    m_wifi.is_scan_done = true;
    break;

  case SYSTEM_EVENT_STA_CONNECTED:
    ESP_LOGI(TAG, "Connected!");
    m_wifi.is_connected = true;
    break;

  case SYSTEM_EVENT_AP_STACONNECTED:
    ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event_connected->mac), event_connected->aid);

    station_cnt++;
    break;

  case SYSTEM_EVENT_AP_STADISCONNECTED:
    ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event_disconnected->mac), event_disconnected->aid);

    station_cnt--;
    break;

  default:
    break;
  }
}

static void m_sys_wifi_softap_expired_callback(void *para)
{
  ESP_LOGI(TAG, "m_sys_wifi_softap_expired_callback!");

  if (station_cnt != 0)
    bsp_tmr_auto_restart(&m_wifi_softap_atm);
  else
    esp_restart();
}

/* End of file -------------------------------------------------------- */
