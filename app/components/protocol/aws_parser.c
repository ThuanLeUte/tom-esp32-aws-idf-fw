/**
* @file       aws_parser.c
* @copyright  Copyright (C) 2021 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2022-02-01
* @author     Thuan Le
* @brief      This module provides all necessary functions 
*             to parse all types of protocol packet in AWS.
* @note       None
* @example    None
*/

/* Includes ----------------------------------------------------------- */
#include "aws_parser.h"
#include "frozen.h"

/* Private defines ---------------------------------------------------- */
static const char *TAG = "aws_parser";

/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
/* Public variables --------------------------------------------------- */
/* Private variables -------------------------------------------------- */
static char data[500] = "";

/* Private function prototypes ---------------------------------------- */
static void scan_array(const char *str, int len, void *user_data);

/* Function definitions ----------------------------------------------- */
bool aws_parse_shadow_packet(sys_aws_shadow_name_t name, const void *buf, uint16_t buf_len, void *p_data)
{
  int res = 0;

  switch (name)
  {
  case SYS_SHADOW_SCALE_TARE:
  {
    uint16_t *scale_tare = p_data;

    res = json_scanf((const char *)buf, (int)buf_len,
                     "{data:{scare_tare:%d}}",
                     scale_tare);

    break;
  }

  default:
    break;
  }

  if (0 == res)
  {
    ESP_LOGW(TAG, "Json parsing fail!");
    return false;
  }

  ESP_LOGI(TAG, "Json parse success!");
  return true;
}

/* Private function definitions --------------------------------------------- */
/**
 * @brief         Parse standard data
 *
 * @param[in]     name        Shadow name
 * @param[in]     buf         Pointer to buffer
 * @param[in]     buf_len     Buffer length
 * @param[in]     p_data      Pointer to data
 *
 * @attention     None
 *
 * @return        None
*/
static void scan_array(const char *str, int len, void *user_data)
{
 
}

/* End of file -------------------------------------------------------- */
