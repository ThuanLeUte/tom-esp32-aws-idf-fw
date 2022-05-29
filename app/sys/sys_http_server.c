/* Simple HTTP Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

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

/* A simple example that demonstrates how to create GET and POST
 * handlers for the web server.
 */
httpd_handle_t server = NULL;

static const char index_html_start[] asm("_binary_index_html_start");
static const char index_html_end[] asm("_binary_index_html_end");

static esp_err_t hello_get_handler(httpd_req_t *req);
static esp_err_t button_post_handler(httpd_req_t *req);

static const char *TAG = "example";

static const httpd_uri_t hello = 
{
    .uri      = "/config",
    .method   = HTTP_GET,
    .handler  = hello_get_handler,
    .user_ctx = NULL
};

static const httpd_uri_t button = 
{
    .uri      = "/setup_btn",
    .method   = HTTP_POST,
    .handler  = button_post_handler,
    .user_ctx = NULL
};

/* An HTTP GET handler */
static esp_err_t hello_get_handler(httpd_req_t *req)
{
  httpd_resp_send(req, index_html_start, index_html_end - index_html_start);

  return ESP_OK;
}

/* An HTTP POST handler */
static esp_err_t button_post_handler(httpd_req_t *req)
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
  httpd_handle_t server = NULL;
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.lru_purge_enable = true;

  // Start the httpd server
  ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
  if (httpd_start(&server, &config) == ESP_OK)
  {
    // Set URI handlers
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &hello);
    httpd_register_uri_handler(server, &button);
    return server;
  }

  ESP_LOGI(TAG, "Error starting server!");
  return NULL;
}

void sys_http_server_stop(void)
{
  httpd_stop(server);
}

void sys_http_server_start(void)
{
  server = start_webserver();
}
