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

/* Private enum/structs ----------------------------------------------------- */
static struct
{
  wifi_config_t config;
  bool is_connected;

} m_wifi =
{
  .is_connected = false,
};

static auto_timer_t m_wifi_softap_atm;
static uint8_t station_cnt = 0;

/* Private defines ---------------------------------------------------------- */
static const char *TAG = "sys_wifi";

#define ESP_WIFI_SSID                   "Lox-Device"
#define ESP_WIFI_PASS                   "123456789"
#define ESP_WIFI_CHANNEL                (1)
#define MAX_STA_CONN                    (1)
// #define SOFT_ACCESS_POINT_WAIT_TIME     (2 * 60 * 1000) // 2 minutes
#define SOFT_ACCESS_POINT_WAIT_TIME     (30 * 1000) // 2 minutes

/* Private variables -------------------------------------------------------- */
/* Private function prototypes ---------------------------------------------- */
static esp_err_t m_sys_wifi_event_handler(void *ctx, system_event_t *event);
static void m_wifi_softap_device_handler(void *arg, esp_event_base_t event_base,
                                         int32_t event_id, void *event_data);
static bool m_sys_wifi_connect(void);
static void m_sys_wifi_softap_expired_callback(void);

/* Function definitions ----------------------------------------------------- */
void sys_wifi_init(void)
{
  // Init TCP/IP stack
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
}

void sys_wifi_sta_init(void)
{
  esp_netif_create_default_wifi_sta();

  // Init ESP WiFi
  wifi_init_config_t wifi_init_cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_cfg));

  // Wifi ssid manager create
  g_ssid_manager = wifi_ssid_manager_create(WIFI_MAX_STATION_NUM);
}

void sys_wifi_softap_init(void)
{
  esp_netif_create_default_wifi_ap();

  // Init ESP WiFi
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                      ESP_EVENT_ANY_ID,
                                                      &m_wifi_softap_device_handler,
                                                      NULL,
                                                      NULL));

  wifi_config_t wifi_config = {
      .ap = {
          .ssid           = ESP_WIFI_SSID,
          .ssid_len       = strlen(ESP_WIFI_SSID),
          .channel        = ESP_WIFI_CHANNEL,
          .password       = ESP_WIFI_PASS,
          .max_connection = MAX_STA_CONN,
          .authmode       = WIFI_AUTH_WPA_WPA2_PSK},
  };

  if (strlen(ESP_WIFI_PASS) == 0)
    wifi_config.ap.authmode = WIFI_AUTH_OPEN;

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  // Start time for wait time
  bsp_tmr_auto_init(&m_wifi_softap_atm, m_sys_wifi_softap_expired_callback);
  bsp_tmr_auto_start(&m_wifi_softap_atm, SOFT_ACCESS_POINT_WAIT_TIME);

  ESP_LOGI(TAG, "Wifi_init_softap finished. SSID:%s Password:%s", ESP_WIFI_SSID, ESP_WIFI_PASS);

  ESP_LOGI(TAG, "Device IP: 192.168.4.1");
}

void sys_wifi_connect(void)
{
  ESP_ERROR_CHECK(esp_event_loop_init(m_sys_wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
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
  // Delete network configs
  ESP_ERROR_CHECK(esp_wifi_get_config(WIFI_IF_STA, &m_wifi.config));

  memset(m_wifi.config.sta.ssid, 0, sizeof(m_wifi.config.sta.ssid));
  memset(m_wifi.config.sta.password, 0, sizeof(m_wifi.config.sta.password));

  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &m_wifi.config));
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
static bool m_sys_wifi_connect(void)
{
  wifi_config_t config = {0};

  wifi_ssid_manager_list_show(g_ssid_manager);

  if (ESP_OK == wifi_ssid_manager_get_best_config(g_ssid_manager, &config))
  {
    ESP_LOGW(TAG, "Selected SSID: %s (%s)", config.sta.ssid, config.sta.password);
    esp_wifi_set_config(WIFI_IF_STA, &config);
    esp_wifi_connect();
    return true;
  }
  else
  {
    ESP_LOGE(TAG, "Can not get the WiFi station in the WiFi save list");
    esp_wifi_connect();
    return false;
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
  switch (event->event_id)
  {
  case SYSTEM_EVENT_STA_START:
  {
    m_sys_wifi_connect();
    break;
  }
  case SYSTEM_EVENT_STA_GOT_IP:
  {
    m_wifi.is_connected = true;
    ESP_LOGE(TAG, "Connected!");
    sys_time_init();

    if (!g_nvs_setting_data.ota.enable)
      sys_aws_init();
    break;
  }
  case SYSTEM_EVENT_STA_DISCONNECTED:
  {
    ESP_LOGE(TAG, "Disconnected!");
    m_wifi.is_connected = false;
    m_sys_wifi_connect(); // WORKAROUND: as ESP32 WiFi libs don't currently auto-reassociate
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
  if (event_id == WIFI_EVENT_AP_STACONNECTED)
  {
    wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
    ESP_LOGI(TAG, "station " MACSTR " join, AID=%d", MAC2STR(event->mac), event->aid);

    station_cnt++;
  }
  else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
  {
    wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
    ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d", MAC2STR(event->mac), event->aid);

    station_cnt--;
  }
}

static void m_sys_wifi_softap_expired_callback(void)
{
  ESP_LOGI(TAG, "m_sys_wifi_softap_expired_callback!");

  if (station_cnt != 0)
    bsp_tmr_auto_restart(&m_wifi_softap_atm);
  else
    esp_restart();
}

/* End of file -------------------------------------------------------- */
