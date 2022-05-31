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

/* Private enum/structs ----------------------------------------------------- */
/* Private defines ---------------------------------------------------------- */
static const char *TAG = "sys_http_server";

/* Private Constants -------------------------------------------------------- */
static const char file_login_html_start[]        asm("_binary_login_html_start");
static const char file_login_html_end[]          asm("_binary_login_html_end");
static const char file_status_html_start[]       asm("_binary_status_html_start");
static const char file_status_html_end[]         asm("_binary_status_html_end");
static const char file_service_html_start[]      asm("_binary_service_html_start");
static const char file_service_html_end[]        asm("_binary_service_html_end");
static const char file_network_html_start[]      asm("_binary_network_html_start");
static const char file_network_html_end[]        asm("_binary_network_html_end");
static const char file_other_html_start[]        asm("_binary_other_html_start");
static const char file_other_html_end[]          asm("_binary_other_html_end");

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
static const char file_service_img_start[]       asm("_binary_service_png_start");
static const char file_service_img_end[]         asm("_binary_service_png_end");
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

/* Private variables -------------------------------------------------------- */
static httpd_handle_t m_server = NULL;

static const httpd_uri_t web_login = 
{
    .uri      = "/login",
    .method   = HTTP_GET,
    .handler  = web_login_html_handler,
    .user_ctx = NULL
};

static const httpd_uri_t web_status = 
{
    .uri      = "/status",
    .method   = HTTP_GET,
    .handler  = web_status_html_handler,
    .user_ctx = NULL
};

static const httpd_uri_t web_network = 
{
    .uri      = "/network",
    .method   = HTTP_GET,
    .handler  = web_network_html_handler,
    .user_ctx = NULL
};

static const httpd_uri_t web_service = 
{
    .uri      = "/service",
    .method   = HTTP_GET,
    .handler  = web_service_html_handler,
    .user_ctx = NULL
};
static const httpd_uri_t web_other = 
{
    .uri      = "/other",
    .method   = HTTP_GET,
    .handler  = web_other_html_handler,
    .user_ctx = NULL
};

static const httpd_uri_t web_button_config = 
{
    .uri      = "/button_config",
    .method   = HTTP_POST,
    .handler  = web_button_config_handler,
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
  httpd_resp_send(req, file_status_html_start, file_status_html_end - file_status_html_start);
  return ESP_OK;
}

static esp_err_t web_service_html_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, file_service_html_start, file_service_html_end - file_service_html_start);
  return ESP_OK;
}

static esp_err_t web_other_html_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, file_other_html_start, file_other_html_end - file_other_html_start);
  return ESP_OK;
}

static esp_err_t web_network_html_handler(httpd_req_t *req)
{
  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, file_network_html_start, file_network_html_end - file_network_html_start);
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
  httpd_resp_send(req, file_service_img_start, file_service_img_end - file_service_img_start);
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
  char *result_ssid= NULL;
  char *result_pwd = NULL;
  char *result_device_id = NULL;

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

  uint8_t res = json_scanf((const char *)buf, (int)sizeof(buf),
                   "{ssid: %Q,password: %Q,device_id: %Q}",
                   &result_ssid, &result_pwd, &result_device_id);

  if (0 == res)
  {
    ESP_LOGW(TAG, "Json parsing fail!");
    return ESP_OK;
  }

  ESP_LOGI(TAG, "Json parse success!");
  ESP_LOGI(TAG, "SSID: %s", result_ssid);
  ESP_LOGI(TAG, "Password: %s", result_pwd);
  ESP_LOGI(TAG, "Device ID: %s", result_device_id);

  strcpy(g_nvs_setting_data.wifi.uiid, result_ssid);
  strcpy(g_nvs_setting_data.wifi.pwd, result_pwd);
  strcpy(g_nvs_setting_data.dev.qr_code, result_device_id);

  if (FLAG_QRCODE_SET_SUCCESS != g_nvs_setting_data.dev.qr_code_flag)
  {
    g_nvs_setting_data.dev.qr_code_flag = FLAG_QRCODE_SET;
  }

  ESP_LOGI(TAG, "SSID after: %s", g_nvs_setting_data.wifi.uiid);
  ESP_LOGI(TAG, "Password after: %s", g_nvs_setting_data.wifi.pwd);
  ESP_LOGI(TAG, "Deivie ID after: %s", g_nvs_setting_data.dev.qr_code);

  // End response
  httpd_resp_send_chunk(req, NULL, 0);

  if ((strlen(g_nvs_setting_data.wifi.uiid) != 0) && (strlen(g_nvs_setting_data.wifi.pwd) != 0))
  {
    SYS_NVS_STORE(wifi);
    SYS_NVS_STORE(dev);
    esp_restart();
  }

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

/* End of file -------------------------------------------------------------- */
