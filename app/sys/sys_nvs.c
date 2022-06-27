/**
 * @file       sys_nvs.c
 * @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
 * @license    This project is released under the Hydratech License.
 * @version    1.0.0
 * @date       2022-02-15
 * @author     Thuan Le
 * @brief      NVS flash memory handle
 * @note       None
 * @example    None
 */

/* Includes ----------------------------------------------------------- */
#include "sys_nvs.h"
#include <stddef.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "sys_aws_provision.h"
#include "bsp.h"
#include "bsp_error.h"

/* Public variables --------------------------------------------------- */
nvs_data_t g_nvs_setting_data;

/* Private defines ---------------------------------------------------- */
#define NVS_STORAGE_SPACENAME    "Storage_1"
#define NVS_VERSION_KEY_NAME     "VERS"

#define NVS_DATA_PAIR(key_id, name)                         \
  { .key = key_id,                                          \
    .offset = offsetof(struct nvs_data_struct, name),       \
    .size = sizeof(g_nvs_setting_data.name)}                \

/* Private enumerate/structure ---------------------------------------- */
typedef struct 
{
  char key[4];         // This is the key-pair of data stored in NVS, we limit it in 4 ASCII number, start from "0000" -> "9999"
  uint32_t offset;     // The offset of variable in @ref nvs_data_struct
  uint32_t size;       // The size of variable in bytes
}
nvs_key_data_t;

const nvs_key_data_t nvs_data_list[] =
{
    NVS_DATA_PAIR("0001", dev)
  , NVS_DATA_PAIR("0002", thing_name)
  , NVS_DATA_PAIR("0003", mac_device_addr)
  , NVS_DATA_PAIR("0004", provision_status)
  , NVS_DATA_PAIR("0005", ota)
  , NVS_DATA_PAIR("0006", wifi)
  , NVS_DATA_PAIR("0007", soft_ap)
  , NVS_DATA_PAIR("0008", properties)
  , NVS_DATA_PAIR("0009", bsp_error)
};

/* Private macros ----------------------------------------------------- */
/* Private Constants -------------------------------------------------------- */
static char *TAG = "sys_nvs";

/* Private variables -------------------------------------------------- */
nvs_handle m_nvs_handle;

/* Private function prototypes ---------------------------------------- */
/* Function definitions ----------------------------------------------- */
void sys_nvs_reset_data(void)
{
  g_nvs_setting_data.data_version          = NVS_DATA_VERSION;
  g_nvs_setting_data.provision_status      = (uint8_t)(AWS_PROVISION_NONE);

  g_nvs_setting_data.properties.sleep_duration = 30;
  g_nvs_setting_data.properties.transmit_delay = 5;
  g_nvs_setting_data.properties.offline_cnt    = 0;
  g_nvs_setting_data.properties.scale_tare     = 0;
  
  memset(&g_nvs_setting_data.wifi, 0, sizeof(g_nvs_setting_data.wifi));

  g_nvs_setting_data.soft_ap.is_change = false;

  sprintf(g_nvs_setting_data.soft_ap.ssid, "%s", "Lox-Device");
  sprintf(g_nvs_setting_data.soft_ap.pwd, "%s", ESP_WIFI_PASS_DEFAULT_AP);
  
  memset(&g_nvs_setting_data.bsp_error, 0, sizeof(g_nvs_setting_data.bsp_error));
}

void sys_nvs_init(void)
{
  uint32_t nvs_ver;
  esp_err_t err;

  // Initialize NVS
  err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    // NVS partition was truncated and needs to be erased, retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }

  if (err != ESP_OK)
    goto _LBL_END_;

  // Open NVS
  err = nvs_open(NVS_STORAGE_SPACENAME, NVS_READWRITE, &m_nvs_handle);
  if (err != ESP_OK)
    goto _LBL_END_;

  // Get NVS data version
  nvs_get_u32(m_nvs_handle, NVS_VERSION_KEY_NAME, &nvs_ver);

  ESP_LOGI(TAG, "Data version in NVS storage: %d ", nvs_ver);
  
  // Check NVS data version
  if (nvs_ver != NVS_DATA_VERSION)
  {
    ESP_LOGI(TAG, "NVS data version is different, all current data in NVS will be erased");

    // Erase NVS storage
    sys_nvs_factory_reset();

    // Set default value to g_nvs_setting_data struct
    sys_nvs_reset_data();

    // Store new data into NVS
    sys_nvs_store_all();

    // Update new NVS data version
    err = nvs_set_u32(m_nvs_handle, NVS_VERSION_KEY_NAME, g_nvs_setting_data.data_version);
    if (err != ESP_OK)
      goto _LBL_END_;

    err = nvs_commit(m_nvs_handle);
    if (err != ESP_OK)
    {
      ESP_LOGE(TAG, "NVS commit error: %s", esp_err_to_name(err));
      goto _LBL_END_;
    }
  }
  else
  {
    // Update NVS data version into RAM
    g_nvs_setting_data.data_version = nvs_ver;

    // Load all data from NVS to RAM structure data
    sys_nvs_load_all();
  }

  return;

_LBL_END_:
  ESP_LOGE(TAG, "NVS storage init faild");
  bsp_error_add(BSP_ERR_NVS_INIT);
}

void sys_nvs_deinit(void)
{
  nvs_close(m_nvs_handle);
}

void sys_nvs_store_all(void)
{
  esp_err_t err;
  uint16_t sizeof_nvs_data_list;
  uint32_t addr;
  void *p_data;
  size_t var_len;

  // Automatically looking into the nvs data list in order to get data information and store to NVS
  sizeof_nvs_data_list = (uint16_t)(sizeof(nvs_data_list) / sizeof(nvs_data_list[0]));
  addr = (uint32_t)&g_nvs_setting_data;

  for (uint_fast16_t i = 0; i < sizeof_nvs_data_list; i++)
  {
    p_data = (void *)(addr + nvs_data_list[i].offset);
    var_len = (size_t)nvs_data_list[i].size;

    err = nvs_set_blob(m_nvs_handle, nvs_data_list[i].key, p_data, var_len);
    if (err != ESP_OK)
    {
      ESP_LOGE(TAG, "NVS set blod error: %s", esp_err_to_name(err));
     goto _LBL_END_;
    }

    // Commit written value
    err = nvs_commit(m_nvs_handle);
    if (err != ESP_OK)
    {
      ESP_LOGE(TAG, "NVS commit error: %s", esp_err_to_name(err));
      goto _LBL_END_;
    }
  }

  return;

_LBL_END_:

  ESP_LOGE(TAG, "NVS store all data error");
  bsp_error_add(BSP_ERR_NVS_COMMUNICATION);
}

void sys_nvs_load_all(void)
{
  esp_err_t err;
  uint16_t sizeof_nvs_data_list;
  uint32_t addr;
  void *p_data;
  size_t  var_len;

  // Load variable data from ID List Table
  addr = (uint32_t)&g_nvs_setting_data;
  sizeof_nvs_data_list = (uint16_t)(sizeof(nvs_data_list) / sizeof(nvs_data_list[0]));

  for (uint_fast16_t i = 0; i < sizeof_nvs_data_list; i++)
  {
    p_data = (void *)(addr + nvs_data_list[i].offset);
    var_len = (size_t)nvs_data_list[i].size;

    err = nvs_get_blob(m_nvs_handle, nvs_data_list[i].key, p_data, &var_len);
    if (err != ESP_OK)
    {
      ESP_LOGE(TAG, "NVS get blod error: %s", esp_err_to_name(err));
      ESP_LOGE(TAG, "NVS load all data error");
      bsp_error_add(BSP_ERR_NVS_COMMUNICATION);
    }
  }
}

void sys_nvs_store(char * p_key_name, void * p_src, uint32_t len)
{
  assert(p_key_name != NULL);
  assert(p_src != NULL);
  esp_err_t err;

  err = nvs_set_blob(m_nvs_handle, p_key_name, p_src, len);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "NVS set blod error: %s", esp_err_to_name(err));
    goto _LBL_END_;
  }

  err = nvs_commit(m_nvs_handle);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "NVS commit error: %s", esp_err_to_name(err));
    goto _LBL_END_;
  }

  return;

_LBL_END_:

  ESP_LOGE(TAG, "NVS store data error");
  bsp_error_add(BSP_ERR_NVS_COMMUNICATION);
}

void sys_nvs_load(char *p_key_name, void *p_des, uint32_t len)
{
  assert(p_key_name != NULL);
  assert(p_des != NULL);
  esp_err_t err;

  err = nvs_get_blob(m_nvs_handle, p_key_name, p_des, (size_t *)&len);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "NVS get blod error: %s", esp_err_to_name(err));
    ESP_LOGE(TAG, "NVS load data error");
    bsp_error_add(BSP_ERR_NVS_COMMUNICATION);
  }
}

void sys_nvs_factory_reset(void)
{
  esp_err_t err;

  err = nvs_erase_all(m_nvs_handle);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "NVS erase all error: %s", esp_err_to_name(err));
    goto _LBL_END_;
  }

  err = nvs_commit(m_nvs_handle);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "NVS commit error: %s", esp_err_to_name(err));
    goto _LBL_END_;
  }

  return;

_LBL_END_:

  ESP_LOGE(TAG, "NVS factory reset failed");
  bsp_error_add(BSP_ERR_NVS_COMMUNICATION);
}

char *sys_nvs_lookup_key(uint32_t offset, uint32_t size)
{
  for (uint_fast16_t i = 0; i < (sizeof(nvs_data_list) / sizeof(nvs_key_data_t)); i++)
  {
    if ((nvs_data_list[i].offset == offset) && (nvs_data_list[i].size == size))
    {
      return (char *)nvs_data_list[i].key;
    }
  }

  // In case there are no key in table, return NULL. Please refer @nvs_data_list
  return NULL;
}

/* End of file -------------------------------------------------------- */
