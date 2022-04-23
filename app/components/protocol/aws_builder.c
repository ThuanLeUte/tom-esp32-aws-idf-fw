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
  ,INFO ( AWS_NOTI_MEASURE_REPORT                   , "measure_report",      1)
  ,INFO ( AWS_NOTI_MEASURE_REPORT_PRESS_YES_TIMEOUT , "measure_report",      1)
  ,INFO ( AWS_NOTI_MEASURE_REPORT_NO_QUES           , "measure_report",      1)
  //    +===========================================+=================+=======+
};
#undef  INFO

/* Private function prototypes ---------------------------------------- */
/* Function definitions ----------------------------------------------- */
void aws_build_packet(aws_packet_type_t type, void *param, void *buf, uint32_t size)
{
}

void aws_build_response(aws_resp_param_t *param, void *buf, uint32_t size)
{
}

void aws_build_notification(aws_noti_param_t *param, void *buf, uint32_t size)
{
}

/* End of file -------------------------------------------------------- */
