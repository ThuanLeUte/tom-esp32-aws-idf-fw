/**
 * @file       bsp_error.c
 * @copyright  Copyright (C) 2021 Hydratech. All rights reserved.
 * @license    This project is released under the Hydratech License.
 * @version    1.0.0
 * @date       2022-01-24
 * @author     Thuan Le
 * @brief      Board Support Error Handler
 * @note       None
 * @example    None
 */

/* Includes ----------------------------------------------------------------- */
#include "bsp.h"
#include "bsp_error.h"
#include "sys_nvs.h"
#include "esp_err.h"

/* Private defines ---------------------------------------------------------- */
static const char *TAG = "bsp_error";

/* Private enumerate/structure ---------------------------------------------- */
/* Public variables --------------------------------------------------------- */
/* Private variables -------------------------------------------------------- */
/* Private function prototypes ---------------------------------------------- */
/* Function definitions ----------------------------------------------------- */
void bsp_error_init(void)
{
  // Reset error structure value if error count equal 0
  if (g_nvs_setting_data.bsp_error.nvs.err_cnt == 0)
    memset(&g_nvs_setting_data.bsp_error, 0, sizeof(g_nvs_setting_data.bsp_error));

  ESP_LOGE(TAG, "Error index: %d", g_nvs_setting_data.bsp_error.nvs.err_idx);
  ESP_LOGE(TAG, "Error count: %d", g_nvs_setting_data.bsp_error.nvs.err_cnt);
}

void bsp_error_add(bsp_error_code_t err)
{
  ESP_LOGE(TAG, "Error add: %d", err);

  if (g_nvs_setting_data.bsp_error.nvs.code[g_nvs_setting_data.bsp_error.nvs.err_idx - 1] == err)
  {
    ESP_LOGE(TAG, "The same with previous error -->ignore");
  }
  else
  {
    g_nvs_setting_data.bsp_error.nvs.code[g_nvs_setting_data.bsp_error.nvs.err_idx] = err;

    g_nvs_setting_data.bsp_error.nvs.err_idx++;
    if (g_nvs_setting_data.bsp_error.nvs.err_idx >= BSP_ERROR_CNT_MAX)
      g_nvs_setting_data.bsp_error.nvs.err_idx = 0;

    if (g_nvs_setting_data.bsp_error.nvs.err_cnt < BSP_ERROR_CNT_MAX)
      g_nvs_setting_data.bsp_error.nvs.err_cnt++;

    // Save error structure to nvs
    bsp_error_sync();
  }
}

void bsp_error_remove(void)
{
  g_nvs_setting_data.bsp_error.nvs.err_cnt--;

  // Save error structure to nvs
  bsp_error_sync();

  ESP_LOGE(TAG, "Error cnt   : %d", g_nvs_setting_data.bsp_error.nvs.err_cnt);
}

uint16_t bsp_error_read_start(void)
{
  g_nvs_setting_data.bsp_error.err_start = (g_nvs_setting_data.bsp_error.nvs.err_cnt >= BSP_ERROR_CNT_MAX) ? g_nvs_setting_data.bsp_error.nvs.err_idx : 0;

  return  g_nvs_setting_data.bsp_error.nvs.err_cnt;
}

uint32_t bsp_error_read(void)
{
  uint32_t err;

  err = g_nvs_setting_data.bsp_error.nvs.code[g_nvs_setting_data.bsp_error.err_start];
  g_nvs_setting_data.bsp_error.err_start++;

  if (g_nvs_setting_data.bsp_error.err_start >= BSP_ERROR_CNT_MAX)
    g_nvs_setting_data.bsp_error.err_start = 0;

  return err;
}

void bsp_error_sync(void)
{
  SYS_NVS_STORE(bsp_error);
}

void bsp_error_esp_to_name(esp_err_t err)
{
  ESP_LOGE(TAG, "Error: %s", esp_err_to_name(err));
}

/* Private function --------------------------------------------------------- */
/* End of file -------------------------------------------------------------- */