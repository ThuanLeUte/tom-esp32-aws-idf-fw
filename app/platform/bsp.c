/**
* @file       bsp.h
* @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2022-24-01
* @author     Thuan Le
* @brief      Board Support Packages
* @note       None
* @example    None
*/

/* Includes ----------------------------------------------------------------- */
#include "bsp.h"
#include "platform_common.h"
#include "esp_spiffs.h"

/* Private defines ---------------------------------------------------------- */
/* Private enumerate/structure ---------------------------------------------- */
/* Private Constants -------------------------------------------------------- */
/* Private variables -------------------------------------------------------- */
/* Private macros ----------------------------------------------------------- */
/* Private Constants -------------------------------------------------------- */
static char *TAG = "bsp";

/* Private prototypes ------------------------------------------------------- */
/* Public APIs -------------------------------------------------------------- */
void bsp_error_handler(bsp_error_t error)
{
  ESP_LOGE(TAG, "BS_ERROR: %d", error);
}

void bsp_delay_ms(uint32_t ms)
{
  vTaskDelay(pdMS_TO_TICKS(ms));
}

void bsp_spiffs_init(void)
{
  esp_err_t ret = ESP_OK;
  ESP_LOGI(TAG, "Initializing SPIFFS");

  esp_vfs_spiffs_conf_t spiffs_init_cfg = 
  {
    .base_path              = "/spiffs",
    .partition_label        = NULL,
    .max_files              = 5,
    .format_if_mount_failed = true
  };
  ret = esp_vfs_spiffs_register(&spiffs_init_cfg);

  if (ESP_OK != ret)
  {
    ESP_LOGE(TAG, "SPIFFS init failed: %s", esp_err_to_name(ret));
    return;
  }

  size_t total = 0, used = 0;
  ret = esp_spiffs_info(NULL, &total, &used);

  if (ESP_OK == ret)
    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
  else
    ESP_LOGE(TAG, "SPIFFS get info failed: %s", esp_err_to_name(ret));
}

/* Private function --------------------------------------------------------- */
/* End of file -------------------------------------------------------------- */
