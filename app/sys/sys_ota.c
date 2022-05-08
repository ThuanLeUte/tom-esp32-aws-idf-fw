/**
 * @file       sys_ota.h
 * @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
 * @license    This project is released under the Hydratech License.
 * @version    1.0.0
 * @date       2020-08-25
 * @author     Thuan Le
 * @brief      System module to handle OTA
 * @note       None
 * @example    None
 */

/* Includes ----------------------------------------------------------------- */
#include "sys_ota.h"
#include "sys_wifi.h"
#include "sys_nvs.h"
#include "bsp_timer.h"
#include "bsp.h"

#include "esp_system.h"
#include "esp_event_loop.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "sys_aws_config.h"
#include "esp_tls.h"
#include "nvs.h"
#include "nvs_flash.h"

/* Private enum/structs ----------------------------------------------------- */
/* Private defines ---------------------------------------------------------- */
static const char *TAG      = "sys/ota";

/* Private function prototypes ---------------------------------------------- */
static esp_err_t m_http_event_handler(esp_http_client_event_t *evt);
static bool m_sys_ota_process(const char *http_url);

/* Private variables -------------------------------------------------------- */
/* Function definitions ----------------------------------------------------- */
void sys_ota_setup(const char *http_url)
{
  if (sys_wifi_is_connected())
  {
    ESP_LOGI(TAG, "Network is connected. OTA");

    g_nvs_setting_data.ota.enable = true;
    strcpy(g_nvs_setting_data.ota.url, http_url);
    SYS_NVS_STORE(ota);

    esp_restart();
  }
  else
  {
    ESP_LOGW(TAG, "Network is not connected. Can not OTA");
  }
}

void sys_ota_start(void)
{
  uint8_t ota_count = 0;
  tmr_t wifi_connect_tmr;

  // Wait wifi stable
  bsp_delay_ms(5000);

  // Wait for wifi is connected {
  bsp_tmr_start(&wifi_connect_tmr, WIFI_CONNECT_TIMEOUT_MS);
  while (!sys_wifi_is_connected())
  {
    bsp_delay_ms(1000);

    if (bsp_tmr_is_expired(&wifi_connect_tmr))
    {
      g_nvs_setting_data.ota.status = OTA_STATE_FAILED;
      break;
    }
  }
  // }

  // Check wifi is connected {
  if (sys_wifi_is_connected())
  {
    ESP_LOGI(TAG, "Network is connected. Enter OTA process");

    // TODO: Led status is white

    ESP_LOGW(TAG, "OTA: %s", g_nvs_setting_data.ota.url);
    while (!m_sys_ota_process(g_nvs_setting_data.ota.url))
    {
      bsp_delay_ms(1000);

      if (ota_count++ >= 3)
      {
        ESP_LOGE(TAG, "Ota failed");
        g_nvs_setting_data.ota.status = OTA_STATE_FAILED;
        break;
      }
    }

    if (ota_count < 3)
    {
      ESP_LOGI(TAG, "Ota succeeded");
      g_nvs_setting_data.ota.status = OTA_STATE_SUCCEEDED;
    }
  }
  else
  {
    ESP_LOGW(TAG, "Network is notconnected. Can not OTA");
    return;
  }
  // }

  // Set ota operation off and restart device
  g_nvs_setting_data.ota.enable = false;
  SYS_NVS_STORE(ota);
  esp_restart();
}

/* Private function --------------------------------------------------------- */
/**
 * @brief         Http event handler
 * 
 * @param[in]     event    Pointer to event
 * 
 * @attention     None
 * 
 * @return
 *    - ESP_OK
 *    - ESP_FAIL
 */
static esp_err_t m_http_event_handler(esp_http_client_event_t *event)
{
  static int curr_len        = 0;
  static int total_length    = 1;
  static int tmp_percentage  = 0;
  static int prev_percentage = 0;

  switch (event->event_id)
  {
  case HTTP_EVENT_ERROR:
    ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
    break;
  case HTTP_EVENT_ON_CONNECTED:
    ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
    break;
  case HTTP_EVENT_HEADER_SENT:
    ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
    break;
  case HTTP_EVENT_ON_HEADER:
    ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", event->header_key, event->header_value);
    if (0 == strcmp(event->header_key, "Content-Length"))
    {
      total_length = atoi(event->header_value);
      ESP_LOGI(TAG, "total length: %d", total_length);
    }
    break;
  case HTTP_EVENT_ON_DATA:
    curr_len += event->data_len;
    tmp_percentage = curr_len * 100 / total_length;
    if (tmp_percentage - prev_percentage >= 5)
    {
      prev_percentage = tmp_percentage;
      if (tmp_percentage >= 0)
      {
        ESP_LOGI(TAG, "Downloading...%d%%", tmp_percentage);
      }
    }
    break;
  case HTTP_EVENT_ON_FINISH:
    ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
    break;
  case HTTP_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
    break;
  }
  return ESP_OK;
}

/**
 * @brief OTA process
 */
static bool m_sys_ota_process(const char *http_url)
{
  ESP_LOGI(TAG, "Starting OTA");

  esp_http_client_config_t config =
  {
    .url           = http_url,
    .event_handler = m_http_event_handler,
  };

  esp_err_t ret = esp_https_ota(&config);
  if (ESP_OK == ret)
    return true;

  ESP_LOGE(TAG, "Firmware upgrade failed");
  return false;
}

/* End of file -------------------------------------------------------------- */
