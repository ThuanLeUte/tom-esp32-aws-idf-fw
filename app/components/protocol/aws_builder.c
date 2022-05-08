/**
* @file       aws_builder.c
* @copyright  Copyright (C) 2021 Hydratech. All rights reserved.
* @license    This project is released under the Hydratech License.
* @version    01.00.00
* @date       2022-02-01
* @author     Thuan Le
* @brief      This module provides all necessary functions 
*             to build all types of protocol packet in AWS.
* @note       None
* @example    None
*/

/* Includes ----------------------------------------------------------- */
#include "aws_builder.h"
#include "frozen.h"
#include "jsmn.h"

/* Private defines ---------------------------------------------------- */
static const char *TAG = "aws_builder";

/* Private enumerate/structure ---------------------------------------- */
/* Private macros ----------------------------------------------------- */
/* Public variables --------------------------------------------------- */
/* Private variables -------------------------------------------------- */
#define INFO(_i, _n)[_i] = { .name = _n}
const aws_req_info_t AWS_REQ_LIST[] =
{
  //    +===============================+=================+
  //    | ID                            | Name            |
  //    +-------------------------------+-----------------+
   INFO ( AWS_REQ_GET_DEVICE_INFO       , "get_dev_info"  )
  //    +===============================+=================+
};

const aws_res_info_t AWS_RES_LIST[] =
{
  //    +===========================+=====================+
  //    | ID                        | Name                |
  //    +---------------------------+---------------------+
   INFO ( AWS_RES_OK                , "ok"                )
  ,INFO ( AWS_RES_BUSY              , "busy"              )
  ,INFO ( AWS_RES_INVALID_PARAM     , "invalid_param"     )
  ,INFO ( AWS_RES_INVALID_OPERATION , "invalid_operation" )
  //    +===========================+=====================+
};
#undef  INFO

#define INFO(_i, _n, _ack)[_i] = { .name = _n, .ack = _ack}
const aws_noti_info_t AWS_NOTI_LIST[] =
{
  //    +===========================================+=================+=======+
  //    | ID                                        | Name            | ACK   |
  //    +-------------------------------------------+-----------------+-------+
   INFO ( AWS_NOTI_ALARM                            , "alarm"         ,      1)
  ,INFO ( AWS_NOTI_DEVICE_DATA                      , "device_data"   ,      1)
  //    +===========================================+=================+=======+
};
#undef  INFO

/* Private function prototypes ---------------------------------------- */
/* Function definitions ----------------------------------------------- */
void aws_build_packet(aws_packet_type_t type, void *param, void *buf, uint32_t size)
{
  switch (type)
  {
  case AWS_PKT_RESP:
    aws_build_response(param, buf, size);
    break;

  case AWS_PKT_NOTI:
    aws_build_notification(param, buf, size);
    break;

  default:
    break;
  }
}

void aws_build_response(aws_resp_param_t *param, void *buf, uint32_t size)
{
}

void aws_build_notification(aws_noti_param_t *param, void *buf, uint32_t size)
{
  #define DEV_DATA param->info.device_data

  struct json_out out = JSON_OUT_BUF(buf, size);

  json_printf(&out, "{type: nt, data:");

  switch (param->noti_type)
  {
  case AWS_NOTI_ALARM:
    json_printf(&out, "{nt: %Q, time: %d, alarm_code: %d}}",
                AWS_NOTI_LIST[param->noti_type].name,
                param->info.time,
                param->info.alarm_code);
    break;

  case AWS_NOTI_DEVICE_DATA:
    ESP_LOGI(TAG, "Noti name       : %s", AWS_NOTI_LIST[param->noti_type].name);
    ESP_LOGI(TAG, "Time            : %d", param->info.time);
    ESP_LOGI(TAG, "Serial number   : %s", DEV_DATA.serial_number);
    ESP_LOGI(TAG, "Weight scale cnt: %d", DEV_DATA.weight_scale_cnt);
    ESP_LOGI(TAG, "Weight scale cnt: %d", DEV_DATA.weight_scale[0]);
    ESP_LOGI(TAG, "Weight scale cnt: %d", DEV_DATA.weight_scale[1]);
    ESP_LOGI(TAG, "Weight scale cnt: %d", DEV_DATA.weight_scale[2]);
    ESP_LOGI(TAG, "Longitude       : %d", DEV_DATA.longitude);
    ESP_LOGI(TAG, "Lattitude       : %d", DEV_DATA.lattitude);

    json_printf(&out, "{nt: %Q, time: %d, serial_number: %Q, weight_scale: %M, longitude: %d, lattitude: %d}}",
                AWS_NOTI_LIST[param->noti_type].name,
                param->info.time,
                DEV_DATA.serial_number,
                json_printf_array, DEV_DATA.weight_scale, DEV_DATA.weight_scale_cnt * sizeof(DEV_DATA.weight_scale[0]), sizeof(DEV_DATA.weight_scale[0]), "%hu",
                DEV_DATA.longitude,
                DEV_DATA.lattitude);

    ESP_LOGI(TAG, "MQTT buffer: %s", (char *)buf);
    
    break;
  
  default:
    break;
  }

  #undef DEV_DATA
}

/* End of file -------------------------------------------------------- */
