/**
* @file       sys_devcfg.c
* @copyright  Copyright (C) 2021 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2020-07-23
* @author     Thuan Le
* @brief      System module to handle device configurations using Blufi protocol
* @note       None
* @example    None
*/
/* Includes ----------------------------------------------------------------- */
#include "sys_devcfg.h"

/* Private defines ---------------------------------------------------------- */
#define AWS_CLIENT_ID_FMT          "ESP32S3%02X%02X%02X%02X%02X%02X"
#define BLUFI_NAME_FMT_EMPTY       "ESP32S3%02x%02x"
#define DEVICE_MAC_ADDR_FMT        "%02X:%02X:%02X:%02X:%02X:%02X"

const char *HEX_CHAR = "0123456789ABCDEF";

// Log tags
static const char *TAG = "BluFi";
const char *TAG_BLUFI_EVT = "[EVENT]";

// Event bit: WiFi connection status
const int CONNECTED_BIT = BIT0;

const char *BLUFI_EVT_STR[] =
{
    "INIT_FINISH"
  , "DEINIT_FINISH"
  , "SET_WIFI_OPMODE"
  , "BLE_CONNECT"
  , "BLE_DISCONNECT"
  , "REQ_CONNECT_TO_AP"
  , "REQ_DISCONNECT_FROM_AP"
  , "GET_WIFI_STATUS"
  , "DEAUTHENTICATE_STA"
  , "RECV_STA_BSSID"
  , "RECV_STA_SSID"
  , "RECV_STA_PASSWD"
  , "RECV_SOFTAP_SSID"
  , "RECV_SOFTAP_PASSWD"
  , "RECV_SOFTAP_MAX_CONN_NUM"
  , "RECV_SOFTAP_AUTH_MODE"
  , "RECV_SOFTAP_CHANNEL"
  , "RECV_USERNAME"
  , "RECV_CA_CERT"
  , "RECV_CLIENT_CERT"
  , "RECV_SERVER_CERT"
  , "RECV_CLIENT_PRIV_KEY"
  , "RECV_SERVER_PRIV_KEY"
  , "RECV_SLAVE_DISCONNECT_BLE"
  , "GET_WIFI_LIST"
  , "REPORT_ERROR"
  , "RECV_CUSTOM_DATA"
};

/* Public variables --------------------------------------------------------- */
wifi_ssid_manager_handle_t g_ssid_manager;

/* Private function prototypes ---------------------------------------------- */
static void       m_blufi_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param);
static void       m_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static esp_err_t  m_wifi_event_handler(void *ctx, system_event_t *event);

/* Private enumerate/structure ---------------------------------------------- */
/* Private variables -------------------------------------------------------- */
static bool m_first_time_wifi_reconnect = true;
static char blufi_name[52];

static uint8_t service_uuid128[32] =
{
  0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xAC, 0xCA, 0x00, 0x00,
};

static esp_ble_adv_data_t adv_data =
{
  .set_scan_rsp        = false,
  .include_name        = true,
  .include_txpower     = true,
  .min_interval        = 30,  // Min connection interval, Time = min_interval * 1.25 msec
  .max_interval        = 60,  // Max connection interval, Time = max_interval * 1.25 msec
  .appearance          = 0x00,
  .manufacturer_len    = 0,
  .p_manufacturer_data = NULL,
  .service_data_len    = 0,
  .p_service_data      = NULL,
  .service_uuid_len    = 16,
  .p_service_uuid      = service_uuid128,
  .flag                = 0x6,
};

static esp_ble_adv_params_t adv_params =
{
  .adv_int_min       = 0x100,
  .adv_int_max       = 0x100,
  .adv_type          = ADV_TYPE_IND,
  .own_addr_type     = BLE_ADDR_TYPE_PUBLIC,
  .channel_map       = ADV_CHNL_ALL,
  .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static esp_blufi_callbacks_t blufi_callbacks =
{
  .event_cb               = m_blufi_event_callback,
  .negotiate_data_handler = blufi_dh_negotiate_data_handler,
  .encrypt_func           = blufi_aes_encrypt,
  .decrypt_func           = blufi_aes_decrypt,
  .checksum_func          = blufi_crc_checksum,
};

// FreeRTOS event group to signal when we are connected & ready to make a request
// static EventGroupHandle_t wifi_event_group;

static struct
{
  struct
  {
    // Store the station info for sending back to phone
    wifi_config_t  config;
    bool           connected;
  }
  wifi;

  struct
  {
    uint8_t   server_if;
    uint16_t  conn_id;
    bool      connected;
  }
  ble;
}
m_mgr = 
{
  .wifi.connected = false,
  .ble.connected  = false
};

/* Function definitions ----------------------------------------------------- */
void sys_devcfg_init(void)
{
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

  // Init WiFi
  ESP_ERROR_CHECK(esp_event_loop_init(m_wifi_event_handler, NULL));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM)); // In this mode, station wakes up to receive beacon every DTIM period

  // Init Bluetooth
  ESP_LOGI(TAG, "BluFi version: %04x\n", esp_blufi_get_version());
  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
  ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
  ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
  ESP_ERROR_CHECK(esp_bluedroid_init());
  ESP_ERROR_CHECK(esp_bluedroid_enable());
  ESP_ERROR_CHECK(esp_ble_gap_register_callback(m_gap_event_handler));

  // Init BLE advertising name
  if (FLAG_QRCODE_SET_SUCCESS != g_nvs_setting_data.dev.qr_code_flag)
  {
    const uint8_t *BT_MAC = esp_bt_dev_get_address();
    ESP_LOGI(TAG, "BT MAC: " DEVICE_MAC_ADDR_FMT, ESP_BD_ADDR_HEX(esp_bt_dev_get_address()));

    sprintf(blufi_name, BLUFI_NAME_FMT_EMPTY, BT_MAC[4], BT_MAC[5]);
  }
  else
  {
    // TODO: Check manufacture data is not correct, maybe due to the ADV name
    sprintf(blufi_name, "%s", g_nvs_setting_data.dev.qr_code);
  }

  // Get MAC ID for AWS
  sprintf(g_nvs_setting_data.mac_device_addr, DEVICE_MAC_ADDR_FMT, ESP_BD_ADDR_HEX(esp_bt_dev_get_address()));
  SYS_NVS_STORE(mac_device_addr);

  ESP_LOGI(TAG, "Device Name: %s", blufi_name);
  ESP_LOGI(TAG, "MAC address: %s", g_nvs_setting_data.mac_device_addr);

  // Init BluFi
  ESP_ERROR_CHECK(esp_blufi_register_callbacks(&blufi_callbacks));
  ESP_ERROR_CHECK(esp_blufi_profile_init());
}

bool sys_devcfg_wifi_connect(void)
{
  wifi_ssid_manager_list_show(g_ssid_manager);
  if (ESP_OK == wifi_ssid_manager_get_best_config(g_ssid_manager, &m_mgr.wifi.config))
  {
    ESP_LOGW(TAG, "Selected SSID: %s (%s)", m_mgr.wifi.config.sta.ssid, m_mgr.wifi.config.sta.password);
    esp_wifi_set_config(ESP_IF_WIFI_STA, &m_mgr.wifi.config);
    esp_wifi_connect();
    return true;
  }
  else
  {
    ESP_LOGE(TAG, "wifi_ssid_manager_get_best_config() failed");
    return false;
  } 
}

/* Private implementations -------------------------------------------------- */
static esp_err_t m_wifi_event_handler(void *ctx, system_event_t *event)
{
  wifi_mode_t mode;

  switch (event->event_id)
  {
  case SYSTEM_EVENT_STA_START:
  {
    esp_wifi_connect();
    break;
  }
  case SYSTEM_EVENT_STA_GOT_IP:
  {
    esp_blufi_extra_info_t info;
  
    m_mgr.wifi.connected = true;
    sys_wifi_set_connection_status(true);

    if (!m_mgr.ble.connected)
    {
      ESP_LOGW(TAG, "BLE NOT CONNECTED\n");
      break; 
    }

    // Wifi manager save ssid and password
    if (strlen((char *) m_mgr.wifi.config.sta.ssid) != 0)
    {
      ESP_LOGW(TAG, "Save wifi ssid     : %s", m_mgr.wifi.config.sta.ssid);
      ESP_LOGW(TAG, "Save wifi password : %s", m_mgr.wifi.config.sta.password);
      wifi_ssid_manager_save(g_ssid_manager, (char *)m_mgr.wifi.config.sta.ssid,
                                             (char *)m_mgr.wifi.config.sta.password);
    }

    // Report WiFi status to App
    memset(&info, 0, sizeof(esp_blufi_extra_info_t));
    memcpy(info.sta_bssid, m_mgr.wifi.config.sta.bssid, 6);
    info.sta_bssid_set = true;
    info.sta_ssid      = m_mgr.wifi.config.sta.ssid;
    info.sta_ssid_len  = strlen((char *) m_mgr.wifi.config.sta.ssid);

    esp_wifi_get_mode(&mode);
    esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, &info);
    
    break;
  }
  case SYSTEM_EVENT_STA_CONNECTED:
  {
    memcpy(m_mgr.wifi.config.sta.bssid, event->event_info.connected.bssid, 6);
    memcpy(m_mgr.wifi.config.sta.ssid, event->event_info.connected.ssid, event->event_info.connected.ssid_len);
    // memcpy(m_mgr.wifi.config.sta.password, event->event_info.connected.password, param->sta_passwd.passwd_len);
    break;
  }
  case SYSTEM_EVENT_STA_DISCONNECTED:
  {
    // Delete WiFi configs
    m_mgr.wifi.connected = false;
    memset(m_mgr.wifi.config.sta.ssid, 0, 32);
    memset(m_mgr.wifi.config.sta.bssid, 0, 6);

    // Send error report
    uint8_t err_code = 0xFF; // Unknown error

    ESP_LOGE(TAG, "disconnect reason: %u\n", event->event_info.disconnected.reason);
    switch (event->event_info.disconnected.reason)
    {
      // ERR: wrong WiFi password
      case WIFI_REASON_MIC_FAILURE:
      case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
      case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
      case WIFI_REASON_IE_IN_4WAY_DIFFERS:
      case WIFI_REASON_HANDSHAKE_TIMEOUT:
      case WIFI_REASON_AUTH_FAIL:
        err_code = 0x10;
        break;
      
      case WIFI_REASON_NO_AP_FOUND:
        err_code = 0x11;
        break;
      
      default: // Other error codes
        break;
    }
    ESP_LOGE(TAG, "err_code: %d\n", err_code);
    esp_blufi_send_error_info((esp_blufi_error_state_t) err_code);

    // Reconnect to network around that connecting before
    if (m_first_time_wifi_reconnect)
    {
      if (!sys_devcfg_wifi_connect())
      {
        err_code = 0xFF;
        esp_blufi_send_error_info((esp_blufi_error_state_t) err_code);
      }
      m_first_time_wifi_reconnect = false;
    }
    else
    {
      ESP_LOGE(TAG, "Failed to reconnect second time");
    }

    break;
  }
  case SYSTEM_EVENT_SCAN_DONE:
  {
    uint16_t ap_cnt = 0;

    // Check if BLE is connected
    if (!m_mgr.ble.connected)
    {
      ESP_LOGW(TAG, "BLE NOT CONNECTED\n");
      break;
    }

    // Get number of APs found
    esp_wifi_scan_get_ap_num(&ap_cnt);
    if (ap_cnt == 0)
    {
      ESP_LOGW(TAG, "No AP found");
      break;
    }

    // Get AP list
    wifi_ap_record_t *ap_list = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * ap_cnt);
    if (!ap_list)
    {
      ESP_LOGE(TAG, "malloc error, ap_list is NULL");
      break;
    }

    // Prepare BluFi AP list
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_cnt, ap_list));
    esp_blufi_ap_record_t *blufi_ap_list = (esp_blufi_ap_record_t *)malloc(ap_cnt * sizeof(esp_blufi_ap_record_t));
    if (!blufi_ap_list)
    {
      free(ap_list);
      ESP_LOGE(TAG, "malloc error, blufi_ap_list is NULL");
      break;
    }

    for (int i = 0; i < ap_cnt; ++i)
    {
      blufi_ap_list[i].rssi = ap_list[i].rssi;
      memcpy(blufi_ap_list[i].ssid, ap_list[i].ssid, sizeof(ap_list[i].ssid));
    }

    // Report WiFi list to App
    esp_blufi_send_wifi_list(ap_cnt, blufi_ap_list);

    // Cleanup
    esp_wifi_scan_stop();
    free(ap_list);
    free(blufi_ap_list);

    break;
  }
  default:
    break;
  }
  return ESP_OK;
}

static void m_blufi_event_callback(esp_blufi_cb_event_t event, esp_blufi_cb_param_t *param)
{
  ESP_LOGW(TAG, "%s %s", TAG_BLUFI_EVT, BLUFI_EVT_STR[event]);

  switch (event)
  {
  case ESP_BLUFI_EVENT_INIT_FINISH:
  {
    SYS_NVS_LOAD(dev);
    
    if (FLAG_QRCODE_SET_SUCCESS == g_nvs_setting_data.dev.qr_code_flag)
    {
      ESP_LOGE(TAG, "QR code: %s", g_nvs_setting_data.dev.qr_code);
      ESP_LOGE(TAG, "QR code length: %d", strlen(g_nvs_setting_data.dev.qr_code));

    // TODO: Check manufacture data is not correct, maybe due to the ADV name
      adv_data.p_manufacturer_data = (uint8_t *)&g_nvs_setting_data.dev.qr_code[2];
      adv_data.manufacturer_len = strlen(g_nvs_setting_data.dev.qr_code) - 2;
    }

    esp_ble_gap_set_device_name(blufi_name);
    esp_ble_gap_config_adv_data(&adv_data);
    break;
  }
  case ESP_BLUFI_EVENT_BLE_CONNECT:
  {
    m_mgr.ble.connected = true;
    m_mgr.ble.server_if = param->connect.server_if;
    m_mgr.ble.conn_id   = param->connect.conn_id;
    esp_ble_gap_stop_advertising();
    blufi_security_init();
    break;
  }
  case ESP_BLUFI_EVENT_BLE_DISCONNECT:
  {
    m_mgr.ble.connected = false;
    blufi_security_deinit();
    esp_ble_gap_start_advertising(&adv_params);

    // Restart after WiFi connected
    if (m_mgr.wifi.connected)
    {
      esp_restart();
    }
    break;
  }
  case ESP_BLUFI_EVENT_SET_WIFI_OPMODE:
  {
    ESP_ERROR_CHECK(esp_wifi_set_mode(param->wifi_mode.op_mode));
    break;
  }
  case ESP_BLUFI_EVENT_REQ_CONNECT_TO_AP:
  {
    // There is no wifi callback when the device has already connected to this wifi
    // so disconnect wifi before connection.
    esp_wifi_disconnect();
    esp_wifi_connect();
    break;
  }
  case ESP_BLUFI_EVENT_REQ_DISCONNECT_FROM_AP:
  {
    esp_wifi_disconnect();
    break;
  }
  case ESP_BLUFI_EVENT_REPORT_ERROR:
  {
    ESP_LOGE(TAG, "err_code: %d\n", param->report_error.state);
    esp_blufi_send_error_info(param->report_error.state);
    break;
  }
  case ESP_BLUFI_EVENT_GET_WIFI_STATUS:
  {
    wifi_mode_t mode;
    esp_blufi_extra_info_t info;

    esp_wifi_get_mode(&mode);

    if (m_mgr.wifi.connected)
    {
      memset(&info, 0, sizeof(esp_blufi_extra_info_t));
      memcpy(info.sta_bssid, m_mgr.wifi.config.sta.bssid, 6);
      info.sta_bssid_set = true;
      info.sta_ssid = m_mgr.wifi.config.sta.ssid;
      info.sta_ssid_len = strlen((char *) m_mgr.wifi.config.sta.ssid);

      esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_SUCCESS, 0, &info);
    }
    else
    {
      esp_blufi_send_wifi_conn_report(mode, ESP_BLUFI_STA_CONN_FAIL, 0, NULL);
    }

    break;
  }
  case ESP_BLUFI_EVENT_RECV_SLAVE_DISCONNECT_BLE:
  {
    esp_blufi_close(m_mgr.ble.server_if, m_mgr.ble.conn_id);
    break;
  }
  case ESP_BLUFI_EVENT_RECV_STA_BSSID:
  {
    memcpy(m_mgr.wifi.config.sta.bssid, param->sta_bssid.bssid, 6);
    m_mgr.wifi.config.sta.bssid_set = 1;
    
    esp_wifi_set_config(WIFI_IF_STA, &m_mgr.wifi.config);
    ESP_LOGW(TAG, "Recv STA BSSID: " ESP_BD_ADDR_STR, ESP_BD_ADDR_HEX(m_mgr.wifi.config.sta.bssid));
    break;
  }
  case ESP_BLUFI_EVENT_RECV_STA_SSID:
  { 
    m_first_time_wifi_reconnect = true;
    strncpy((char *)m_mgr.wifi.config.sta.ssid, (char *)param->sta_ssid.ssid, param->sta_ssid.ssid_len);
    m_mgr.wifi.config.sta.ssid[param->sta_ssid.ssid_len] = '\0';

    esp_wifi_set_config(WIFI_IF_STA, &m_mgr.wifi.config);
    ESP_LOGW(TAG, "Recv STA SSID: %s\n", m_mgr.wifi.config.sta.ssid);
    break;
  }
  case ESP_BLUFI_EVENT_RECV_STA_PASSWD:
  {
    strncpy((char *)m_mgr.wifi.config.sta.password, (char *)param->sta_passwd.passwd, param->sta_passwd.passwd_len);
    m_mgr.wifi.config.sta.password[param->sta_passwd.passwd_len] = '\0';

    esp_wifi_set_config(WIFI_IF_STA, &m_mgr.wifi.config);
    ESP_LOGW(TAG, "Recv STA password %s\n", m_mgr.wifi.config.sta.password);
    break;
  }
  case ESP_BLUFI_EVENT_GET_WIFI_LIST:
  {
    // TODO: TEST NEW WiFi scan configs
    wifi_scan_config_t scan_cfg = 
    {
      .ssid        = NULL,
      .bssid       = NULL,
      .channel     = 0,
      .show_hidden = true,
      .scan_type   = WIFI_SCAN_TYPE_ACTIVE
    };
    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_cfg, true));

    break;
  }
  case ESP_BLUFI_EVENT_RECV_CUSTOM_DATA:
  {
    SYS_NVS_LOAD(dev);

    // Check Device ID is set or not
    if (FLAG_QRCODE_SET_SUCCESS != g_nvs_setting_data.dev.qr_code_flag)
    {
      g_nvs_setting_data.dev.qr_code_flag = FLAG_QRCODE_SET;
      strncpy(g_nvs_setting_data.dev.qr_code, (const char *)param->custom_data.data, param->custom_data.data_len);
    
      ESP_LOGI(TAG, "QR code is not set. Set Device ID: %s", g_nvs_setting_data.dev.qr_code);
    }
    else
    {
      ESP_LOGI(TAG, "QR code is already set successfull");
    }

    break;
  }

  //! CURRENTLY NOT SUPPORTED
  case ESP_BLUFI_EVENT_DEINIT_FINISH:
  case ESP_BLUFI_EVENT_DEAUTHENTICATE_STA:
  case ESP_BLUFI_EVENT_RECV_SOFTAP_SSID:
  case ESP_BLUFI_EVENT_RECV_SOFTAP_PASSWD:
  case ESP_BLUFI_EVENT_RECV_SOFTAP_MAX_CONN_NUM:
  case ESP_BLUFI_EVENT_RECV_SOFTAP_AUTH_MODE:
  case ESP_BLUFI_EVENT_RECV_SOFTAP_CHANNEL:
  case ESP_BLUFI_EVENT_RECV_USERNAME:
  case ESP_BLUFI_EVENT_RECV_CA_CERT:
  case ESP_BLUFI_EVENT_RECV_CLIENT_CERT:
  case ESP_BLUFI_EVENT_RECV_SERVER_CERT:
  case ESP_BLUFI_EVENT_RECV_CLIENT_PRIV_KEY:
  case ESP_BLUFI_EVENT_RECV_SERVER_PRIV_KEY:
  default:
    break;
  }
}

static void m_gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
  switch (event)
  {
  case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
    esp_ble_gap_start_advertising(&adv_params);
    break;
  default:
    break;
  }
}

/* End of file -------------------------------------------------------------- */
