/**
* @file       sys_http_server.c
* @copyright  Copyright (C) 2021 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2021-08-13
* @author     Thuan Le
* @brief      System module to handle HTTP Server
* @note       None
* @example    None
*/

/* Includes ----------------------------------------------------------------- */
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_tls_crypto.h"
#include <esp_http_server.h>
#include "frozen.h"
#include "sys_nvs.h"
#include "sys_devcfg.h"
#include "bsp.h"

/* Private enum/structs ----------------------------------------------------- */
/* Private defines ---------------------------------------------------------- */
static const char *TAG = "sys_http_server";

/* Private Constants -------------------------------------------------------- */
static const char file_login_html_start[]        asm("_binary_login_html_start");
static const char file_login_html_end[]          asm("_binary_login_html_end");
static const char file_system_html_start[]       asm("_binary_system_html_start");
static const char file_system_html_end[]         asm("_binary_system_html_end");
static const char file_properties_html_start[]   asm("_binary_properties_html_start");
static const char file_properties_html_end[]     asm("_binary_properties_html_end");
static const char file_ap_html_start[]           asm("_binary_ap_html_start");
static const char file_ap_html_end[]             asm("_binary_ap_html_end");
static const char file_reboot_html_start[]       asm("_binary_reboot_html_start");
static const char file_reboot_html_end[]         asm("_binary_reboot_html_end");

static const char file_bootstrap_min_css_start[] asm("_binary_bootstrap_min_css_start");
static const char file_bootstrap_min_css_end[]   asm("_binary_bootstrap_min_css_end");
static const char file_pixie_main_css_start[]    asm("_binary_pixie_main_css_start");
static const char file_pixie_main_css_end[]      asm("_binary_pixie_main_css_end");

static const char file_bootstrap_min_js_start[]  asm("_binary_bootstrap_min_js_start");
static const char file_bootstrap_min_js_end[]    asm("_binary_bootstrap_min_js_end");
static const char file_pixie_custom_js_start[]   asm("_binary_pixie_custom_js_start");
static const char file_pixie_custom_js_end[]     asm("_binary_pixie_custom_js_end");
static const char file_jquery_js_start[]         asm("_binary_jquery_min_js_start");
static const char file_jquery_js_end[]           asm("_binary_jquery_min_js_end");

static const char file_ap_img_start[]            asm("_binary_ap_png_start");
static const char file_ap_img_end[]              asm("_binary_ap_png_end");
static const char file_eye_close_img_start[]     asm("_binary_eye_close_png_start");
static const char file_eye_close_img_end[]       asm("_binary_eye_close_png_end");
static const char file_light_img_start[]         asm("_binary_light_png_start");
static const char file_light_img_end[]           asm("_binary_light_png_end");
static const char file_network_img_start[]       asm("_binary_network_png_start");
static const char file_network_img_end[]         asm("_binary_network_png_end");
static const char file_other_img_start[]         asm("_binary_other_png_start");
static const char file_other_img_end[]           asm("_binary_other_png_end");
static const char file_peripheral_img_start[]    asm("_binary_periperal_png_start");
static const char file_peripheral_img_end[]      asm("_binary_periperal_png_end");
static const char file_reboot_img_start[]        asm("_binary_reboot_png_start");
static const char file_reboot_img_end[]          asm("_binary_reboot_png_end");
static const char file_properties_img_start[]       asm("_binary_service_png_start");
static const char file_properties_img_end[]         asm("_binary_service_png_end");
static const char file_status_img_start[]        asm("_binary_status_png_start");
static const char file_status_img_end[]          asm("_binary_status_png_end");
static const char file_timezone_img_start[]      asm("_binary_timezone_png_start");
static const char file_timezone_img_end[]        asm("_binary_timezone_png_end");
static const char file_upgrade_img_start[]       asm("_binary_upgrade_png_start");
static const char file_upgrade_img_end[]         asm("_binary_upgrade_png_end");

/* Public variables --------------------------------------------------------- */
/* Private function prototypes ---------------------------------------------- */
static httpd_handle_t start_webserver(void);

static esp_err_t web_login_html_handler(httpd_req_t *req);
static esp_err_t web_status_html_handler(httpd_req_t *req);
static esp_err_t web_service_html_handler(httpd_req_t *req);
static esp_err_t web_other_html_handler(httpd_req_t *req);
static esp_err_t web_network_html_handler(httpd_req_t *req);

static esp_err_t web_button_config_handler(httpd_req_t *req);
static esp_err_t web_esp32_data_handler(httpd_req_t *req);
static esp_err_t web_esp32_wifi_handler(httpd_req_t *req);
static esp_err_t web_esp32_wifi_list_handler(httpd_req_t *req);
static esp_err_t web_esp32_device_properties_handler(httpd_req_t *req);

static esp_err_t web_bootstrap_min_css_handler(httpd_req_t *req);
static esp_err_t web_pixie_main_css_handler(httpd_req_t *req);

static esp_err_t web_bootstrap_min_js_handler(httpd_req_t *req);
static esp_err_t web_pixie_custom_js_handler(httpd_req_t *req);
static esp_err_t web_jquery_js_handler(httpd_req_t *req);

static esp_err_t web_ap_img_handler(httpd_req_t *req);
static esp_err_t web_eye_close_img_handler(httpd_req_t *req);
static esp_err_t web_light_img_handler(httpd_req_t *req);
static esp_err_t web_network_img_handler(httpd_req_t *req);
static esp_err_t web_other_img_handler(httpd_req_t *req);
static esp_err_t web_peripheral_img_handler(httpd_req_t *req);
static esp_err_t web_reboot_img_handler(httpd_req_t *req);
static esp_err_t web_service_img_handler(httpd_req_t *req);
static esp_err_t web_status_img_handler(httpd_req_t *req);
static esp_err_t web_timezone_img_handler(httpd_req_t *req);
static esp_err_t web_upgrade_img_handler(httpd_req_t *req);

static bool sys_http_server_handle_data(char *buf, int buf_len);

/* Private variables -------------------------------------------------------- */
static httpd_handle_t m_server = NULL;
static char *sta_ssid = NULL;
static char *sta_password = NULL;

static const httpd_uri_t web_login = 
{
    .uri      = "/login",
    .method   = HTTP_GET,
    .handler  = web_login_html_handler,
    .user_ctx = NULL
};

static const httpd_uri_t web_status = 
{
    .uri      = "/system",
    .method   = HTTP_GET,
    .handler  = web_status_html_handler,
    .user_ctx = NULL
};

static const httpd_uri_t web_network = 
{
    .uri      = "/ap",
    .method   = HTTP_GET,
    .handler  = web_network_html_handler,
    .user_ctx = NULL
};

static const httpd_uri_t web_service = 
{
    .uri      = "/properties",
    .method   = HTTP_GET,
    .handler  = web_service_html_handler,
    .user_ctx = NULL
};
static const httpd_uri_t web_other = 
{
    .uri      = "/reboot",
    .method   = HTTP_GET,
    .handler  = web_other_html_handler,
    .user_ctx = NULL
};

static const httpd_uri_t web_button_config = 
{
    .uri      = "/button_handler",
    .method   = HTTP_POST,
    .handler  = web_button_config_handler,
    .user_ctx = NULL
};

static const httpd_uri_t web_esp32_data = 
{
    .uri      = "/esp32_data",
    .method   = HTTP_GET,
    .handler  = web_esp32_data_handler,
    .user_ctx = NULL
};

static const httpd_uri_t web_esp32_wifi_status = 
{
    .uri      = "/esp32_wifi_status",
    .method   = HTTP_GET,
    .handler  = web_esp32_wifi_handler,
    .user_ctx = NULL
};

static const httpd_uri_t web_esp32_wifi_list = 
{
    .uri      = "/esp32_wifi_list",
    .method   = HTTP_GET,
    .handler  = web_esp32_wifi_list_handler,
    .user_ctx = NULL
};

static const httpd_uri_t web_esp32_device_properties = 
{
    .uri      = "/esp32_properties",
    .method   = HTTP_GET,
    .handler  = web_esp32_device_properties_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_bootstrap_min_css = 
{
    .uri      = "/web/css/bootstrap.min.css",
    .method   = HTTP_GET,
    .handler  = web_bootstrap_min_css_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_pixie_main_css =
{
    .uri      = "/web/css/pixie-main.css",
    .method   = HTTP_GET,
    .handler  = web_pixie_main_css_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_bootstrap_min_js = 
{
    .uri      = "/web/js/bootstrap.min.js",
    .method   = HTTP_GET,
    .handler  = web_bootstrap_min_js_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_pixie_custom_js =
{
    .uri      = "/web/js/pixie-custom.js",
    .method   = HTTP_GET,
    .handler  = web_pixie_custom_js_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_jquery_js = 
{
    .uri      = "/web/js/jquery.min.js",
    .method   = HTTP_GET,
    .handler  = web_jquery_js_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_ap_img = 
{
    .uri      = "/web/images/ap.png",
    .method   = HTTP_GET,
    .handler  = web_ap_img_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_eye_close_img = 
{
    .uri      = "/web/images/eye-close.png",
    .method   = HTTP_GET,
    .handler  = web_eye_close_img_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_light_img = 
{
    .uri      = "/web/images/light.png",
    .method   = HTTP_GET,
    .handler  = web_light_img_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_network_img = 
{
    .uri      = "/web/images/network.png",
    .method   = HTTP_GET,
    .handler  = web_network_img_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_other_img = 
{
    .uri      = "/web/images/other.png",
    .method   = HTTP_GET,
    .handler  = web_other_img_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_peripheral_img = 
{
    .uri      = "/web/images/peripheral.png",
    .method   = HTTP_GET,
    .handler  = web_peripheral_img_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_reboot_img = 
{
    .uri      = "/web/images/reboot.png",
    .method   = HTTP_GET,
    .handler  = web_reboot_img_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_service_img = 
{
    .uri      = "/web/images/service.png",
    .method   = HTTP_GET,
    .handler  = web_service_img_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_status_img = 
{
    .uri      = "/web/images/status.png",
    .method   = HTTP_GET,
    .handler  = web_status_img_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_timezone_img = 
{
    .uri      = "/web/images/timezone.png",
    .method   = HTTP_GET,
    .handler  = web_timezone_img_handler,
    .user_ctx = NULL
};

static httpd_uri_t web_upgrade_img = 
{
    .uri      = "/web/images/upgrade.png",
    .method   = HTTP_GET,
    .handler  = web_upgrade_img_handler,
    .user_ctx = NULL
};

/* Function definitions ----------------------------------------------------- */
void sys_http_server_stop(void)
{
  httpd_stop(m_server);
}

void sys_http_server_start(void)
{
  m_server = start_webserver();
}

/* Private function definitions --------------------------------------------- */
static esp_err_t web_bootstrap_min_css_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/css");
  httpd_resp_send(req, (const char *)file_bootstrap_min_css_start, file_bootstrap_min_css_end - file_bootstrap_min_css_start);
  return ESP_OK;
}

static esp_err_t web_pixie_main_css_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/css");
  httpd_resp_send(req, (const char *)file_pixie_main_css_start, file_pixie_main_css_end - file_pixie_main_css_start);
  return ESP_OK;
}

static esp_err_t web_login_html_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, file_login_html_start, file_login_html_end - file_login_html_start);
  return ESP_OK;
}

static esp_err_t web_status_html_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, file_system_html_start, file_system_html_end - file_system_html_start);
  return ESP_OK;
}

static esp_err_t web_esp32_data_handler(httpd_req_t *req)
{
  char buf[300];

  struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));

  json_printf(&out, "{type: system, mac: %Q, ap_ip: %Q, wifi_network: %Q, station_ip: %Q, active_service: %Q}",
              g_nvs_setting_data.mac_device_addr,
              "192.168.4.1",
              g_nvs_setting_data.soft_ap.ssid,
              "192.168.4.2",
              "WiFi");

  httpd_resp_send(req, buf, strlen(buf));
  return ESP_OK;
}

static esp_err_t web_esp32_wifi_handler(httpd_req_t *req)
{
  char buf_wifi[100];

  struct json_out out = JSON_OUT_BUF(buf_wifi, sizeof(buf_wifi));

  json_printf(&out, "{type: wifi_status, status: %d}",
              sys_wifi_is_connected());
  
  // Check and save WiFi information
  if (sys_wifi_is_connected())
  {
    SYS_NVS_STORE(wifi);
    wifi_ssid_manager_save(g_ssid_manager, (char *)sta_ssid, (char *)sta_password);
  }

  httpd_resp_send(req, buf_wifi, strlen(buf_wifi));
  return ESP_OK;
}

static esp_err_t web_esp32_wifi_list_handler(httpd_req_t *req)
{
  static char buf_list[2000];

  if (sys_wifi_is_scan_done())
  {
    sys_wifi_get_scan_wifi_list(buf_list, sizeof(buf_list));
    sys_wifi_set_wifi_scan_status(false);
  }

  httpd_resp_send(req, buf_list, strlen(buf_list));
  return ESP_OK;
}

static esp_err_t web_esp32_device_properties_handler(httpd_req_t *req)
{
  char buf[200];

  struct json_out out = JSON_OUT_BUF(buf, sizeof(buf));

  json_printf(&out, "{type: properties, hw_ver_id: %Q, fw_ver_id: %Q, sleep_duration: %d, transmit_duration: %d, offline_cnt: %d, tare_value: %d}",
              DEVICE_FIRMWARE_VERSION,
              DEVICE_HARDWARE_VERSION,
              g_nvs_setting_data.properties.sleep_duration,
              g_nvs_setting_data.properties.transmit_delay,
              g_nvs_setting_data.properties.offline_cnt,
              g_nvs_setting_data.properties.scale_tare);

  httpd_resp_send(req, buf, strlen(buf));
  return ESP_OK;
}

static esp_err_t web_service_html_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, file_properties_html_start, file_properties_html_end - file_properties_html_start);
  return ESP_OK;
}

static esp_err_t web_other_html_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, file_reboot_html_start, file_reboot_html_end - file_reboot_html_start);
  return ESP_OK;
}

static esp_err_t web_network_html_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, file_ap_html_start, file_ap_html_end - file_ap_html_start);
  return ESP_OK;
}

static esp_err_t web_bootstrap_min_js_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/javascript");
  httpd_resp_send(req, file_bootstrap_min_js_start, file_bootstrap_min_js_end - file_bootstrap_min_js_start);
  return ESP_OK;
}

static esp_err_t web_pixie_custom_js_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/javascript");
  httpd_resp_send(req, file_pixie_custom_js_start, file_pixie_custom_js_end - file_pixie_custom_js_start);
  return ESP_OK;
}

static esp_err_t web_jquery_js_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/javascript");
  httpd_resp_send(req, file_jquery_js_start, file_jquery_js_end - file_jquery_js_start);
  return ESP_OK;
}

static esp_err_t web_ap_img_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "image/png");
  httpd_resp_send(req, file_ap_img_start, file_ap_img_end - file_ap_img_start);
  return ESP_OK;
}

static esp_err_t web_eye_close_img_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "image/png");
  httpd_resp_send(req, file_eye_close_img_start, file_eye_close_img_end - file_eye_close_img_start);
  return ESP_OK;
}

static esp_err_t web_light_img_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "image/png");
  httpd_resp_send(req, file_light_img_start, file_light_img_end - file_light_img_start);
  return ESP_OK;
}

static esp_err_t web_network_img_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "image/png");
  httpd_resp_send(req, file_network_img_start, file_network_img_end - file_network_img_start);
  return ESP_OK;
}

static esp_err_t web_other_img_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "image/png");
  httpd_resp_send(req, file_other_img_start, file_other_img_end - file_other_img_start);
  return ESP_OK;
}

static esp_err_t web_peripheral_img_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "image/png");
  httpd_resp_send(req, file_peripheral_img_start, file_peripheral_img_end - file_peripheral_img_start);
  return ESP_OK;
}

static esp_err_t web_reboot_img_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "image/png");
  httpd_resp_send(req, file_reboot_img_start, file_reboot_img_end - file_reboot_img_start);
  return ESP_OK;
}

static esp_err_t web_service_img_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "image/png");
  httpd_resp_send(req, file_properties_img_start, file_properties_img_end - file_properties_img_start);
  return ESP_OK;
}

static esp_err_t web_status_img_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "image/png");
  httpd_resp_send(req, file_status_img_start, file_status_img_end - file_status_img_start);
  return ESP_OK;
}

static esp_err_t web_timezone_img_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "image/png");
  httpd_resp_send(req, file_timezone_img_start, file_timezone_img_end - file_timezone_img_start);
  return ESP_OK;
}

static esp_err_t web_upgrade_img_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "image/png");
  httpd_resp_send(req, file_upgrade_img_start, file_upgrade_img_end - file_upgrade_img_start);
  return ESP_OK;
}

static esp_err_t web_button_config_handler(httpd_req_t *req)
{
  char buf[500];
  int ret = 0;
  int remaining = req->content_len;

  memset(buf, 0, 500);

  while (remaining > 0)
  {
    // Read the data for the request
    if ((ret = httpd_req_recv(req, buf,
                              MIN(remaining, sizeof(buf)))) <= 0)
    {
      if (ret == HTTPD_SOCK_ERR_TIMEOUT)
      {
        // Retry receiving if timeout occurred
        continue;
      }
      return ESP_FAIL;
    }

    // Send back the same data
    httpd_resp_send_chunk(req, buf, ret);
    remaining -= ret;
  }

  // Log data received
  ESP_LOGI(TAG, "%.*s", ret, buf);

  if (!sys_http_server_handle_data(buf, sizeof(buf)))
  {
    ESP_LOGE(TAG, "Parsing data failed");
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "Parsing data success");

  // End response
  httpd_resp_send_chunk(req, NULL, 0);

  return ESP_OK;
}

static httpd_handle_t start_webserver(void)
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;
  config.max_uri_handlers = 50;

  // Start the httpd server
  ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
  if (httpd_start(&m_server, &config) == ESP_OK)
  {
    // Set URI handlers
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(m_server, &web_login);
    httpd_register_uri_handler(m_server, &web_status);
    httpd_register_uri_handler(m_server, &web_service);
    httpd_register_uri_handler(m_server, &web_network);
    httpd_register_uri_handler(m_server, &web_other);

    httpd_register_uri_handler(m_server, &web_button_config);
    httpd_register_uri_handler(m_server, &web_esp32_data);
    httpd_register_uri_handler(m_server, &web_esp32_wifi_status);
    httpd_register_uri_handler(m_server, &web_esp32_wifi_list);
    httpd_register_uri_handler(m_server, &web_esp32_device_properties);

    httpd_register_uri_handler(m_server, &web_bootstrap_min_css);
    httpd_register_uri_handler(m_server, &web_pixie_main_css);

    httpd_register_uri_handler(m_server, &web_bootstrap_min_js);
    httpd_register_uri_handler(m_server, &web_pixie_custom_js);
    httpd_register_uri_handler(m_server, &web_jquery_js);

    httpd_register_uri_handler(m_server, &web_ap_img);
    httpd_register_uri_handler(m_server, &web_eye_close_img);
    httpd_register_uri_handler(m_server, &web_light_img);
    httpd_register_uri_handler(m_server, &web_network_img);
    httpd_register_uri_handler(m_server, &web_other_img);
    httpd_register_uri_handler(m_server, &web_peripheral_img);
    httpd_register_uri_handler(m_server, &web_reboot_img);
    httpd_register_uri_handler(m_server, &web_service_img);
    httpd_register_uri_handler(m_server, &web_status_img);
    httpd_register_uri_handler(m_server, &web_timezone_img);
    httpd_register_uri_handler(m_server, &web_upgrade_img);

    return m_server;
  }

  ESP_LOGI(TAG, "Error starting server!");
  return NULL;
}

static bool sys_http_server_handle_data(char *buf, int buf_len)
{
  uint8_t res;
  char *operation      = NULL;
  char *app_ssid       = NULL;
  char *app_password   = NULL;

  char *sleep_duration = NULL;
  char *transmit_delay = NULL;
  char *offline_cnt    = NULL;
  char *tare_value     = NULL;

  res = json_scanf((const char *)buf, (int)buf_len,
                   "{operation: %Q}",
                   &operation);

  ESP_LOGI(TAG, "Operation: %s", operation);

  if (strcmp("system", operation) == 0)
  {
    res = json_scanf((const char *)buf, (int)buf_len,
                     "{operation: %Q, ap_ssid: %Q, ap_password: %Q}",
                     &operation, &app_ssid, &app_password);

    sprintf(g_nvs_setting_data.soft_ap.ssid, "%s", app_ssid);
    sprintf(g_nvs_setting_data.soft_ap.pwd, "%s", app_password);
    g_nvs_setting_data.soft_ap.is_change = true;
    
    ESP_LOGI(TAG, "SSID:%s Password:%s", g_nvs_setting_data.soft_ap.ssid, g_nvs_setting_data.soft_ap.pwd);

    if ((strlen(g_nvs_setting_data.soft_ap.pwd) >= 8) && (strlen(g_nvs_setting_data.soft_ap.ssid) > 0))
    {
      ESP_LOGI(TAG, "Store to NVS SAP");
      SYS_NVS_STORE(soft_ap);
    }
    else
    {
      ESP_LOGE(TAG, "The ssid empty or password less than 8");
    }
  }
  else if (strcmp("network_scan_wifi", operation) == 0)
  {
    sys_wifi_scan_start();
  }
  else if (strcmp("network_connect", operation) == 0)
  {
    res = json_scanf((const char *)buf, (int)buf_len,
                     "{operation: %Q, sta_ssid: %Q, sta_password: %Q}",
                     &operation, &sta_ssid, &sta_password);

    ESP_LOGI(TAG, "Connect to SSID: %s, Password: %s", sta_ssid, sta_password);

    sys_wifi_connect(sta_ssid, sta_password);

    strcpy(g_nvs_setting_data.wifi.uiid, (const char *)sta_ssid);
    strcpy(g_nvs_setting_data.wifi.pwd, (const char *)sta_password);
  }
  else if (strcmp("properties_apply", operation) == 0)
  {
    res = json_scanf((const char *)buf, (int)buf_len,
                     "{operation: %Q, sleep_duration: %Q, transmit_delay: %Q, offline_cnt: %Q, tare_value: %Q}",
                     &operation,
                     &sleep_duration, &transmit_delay, &offline_cnt, &tare_value);

    ESP_LOGI(TAG, "Sleep duration: %s", sleep_duration);
    ESP_LOGI(TAG, "Transmit delay: %s", transmit_delay);
    ESP_LOGI(TAG, "Offline count : %s", offline_cnt);
    ESP_LOGI(TAG, "Tare vale     : %s", tare_value);

    g_nvs_setting_data.properties.sleep_duration = atoi(sleep_duration);
    g_nvs_setting_data.properties.transmit_delay = atoi(transmit_delay);
    g_nvs_setting_data.properties.offline_cnt    = atoi(offline_cnt);
    g_nvs_setting_data.properties.scale_tare     = atoi(tare_value);

    SYS_NVS_STORE(properties);
  }
  else if (strcmp("reboot", operation) == 0)
  {
    esp_restart();
  }

  if (0 == res)
    return false;

  return true;
}

/* End of file -------------------------------------------------------------- */
