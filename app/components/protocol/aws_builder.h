/**
* @file       aws_builder.h
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

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __AWS_BUILDER_H
#define __AWS_BUILDER_H

/* Includes ----------------------------------------------------------- */
#include "platform_common.h"

/* Public defines ----------------------------------------------------- */
/* Public enumerate/structure ----------------------------------------- */
/**
 * @brief AWS packet type enum
 */
typedef enum
{
  AWS_PKT_RESP  = 0x00 // Response
 ,AWS_PKT_NOTI  = 0x01 // Notification
 ,AWS_PKT_ACK   = 0x02 // Acknowledgement
 
 ,AWS_PKT_UNKNOWN
}
aws_packet_type_t;

typedef enum
{
   QUES_YES = 0
  ,QUES_NO
  ,QUES_SKIP
  ,QUES_TIMEOUT
}
sys_ques_status_t;

/**
 * @brief AWS result type
 */
typedef enum
{
   AWS_RES_OK
  ,AWS_RES_BUSY
  ,AWS_RES_INVALID_PARAM
  ,AWS_RES_INVALID_OPERATION
 
  ,AWS_RES_UNKNOWN
}
aws_res_type_t;

/**
 * @brief AWS request type
 */
typedef enum
{
   AWS_REQ_GET_DEVICE_INFO
 
  ,AWS_REQ_UNKNOWN
}
aws_req_type_t;

/**
 * @brief AWS notification type
 */
typedef enum
{
   AWS_NOTI_ALARM
  ,AWS_NOTI_MEASURE_REPORT
  ,AWS_NOTI_MEASURE_REPORT_PRESS_YES_TIMEOUT
  ,AWS_NOTI_MEASURE_REPORT_NO_QUES
 
  ,AWS_NOTI_UNKNOWN
}
aws_noti_type_t;

/**
 * @brief AWS notification info
 */
typedef struct
{
  bool ack;
  char * const name;
}
aws_noti_info_t;

/**
 * @brief AWS request info
 */
typedef struct
{
  char * const name;
}
aws_req_info_t;

/**
 * @brief AWS result info
 */
typedef struct
{
  char * const name;
}
aws_res_info_t;

/**
 * @brief AWS response param info
 */
typedef struct
{
  struct
  {
    char hw[10];
    char fw[10];
  }
  dev_info;
}
aws_resp_param_info_t;

/**
 * @brief AWS notification param info
 */
typedef struct
{
  char patient_id[100];
  uint32_t time;

  struct
  {
    float temp;
    uint8_t spo2;
    uint8_t hr;

    uint32_t question_id[100];
    sys_ques_status_t question_status[100];
    uint16_t question_cnt;
  }
  measure_report;
}
aws_noti_param_info_t;

/**
 * @brief AWS response param
 */
typedef struct
{
  aws_req_type_t req_type;
  aws_res_type_t res_type;

  aws_resp_param_info_t info;
}
aws_resp_param_t;

/**
 * @brief AWS notification param
 */
typedef struct
{
  aws_noti_type_t noti_type;
  uint32_t noti_id;

  aws_noti_param_info_t info;
}
aws_noti_param_t;

/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */
/**
 * @brief         AWS build packet
 *
 * @param[in]     type    Packet type
 * @param[in]     param   Pointer to packet param
 * @param[out]    buf     Point to paket buf
 * @param[in]     size    Size of packet buf
 *
 * @attention     None
 *
 * @return        None
 */
void aws_build_packet(aws_packet_type_t type, void *param, void *buf, uint32_t size);

/**
 * @brief         AWS build response packet
 *
 * @param[in]     param   Pointer to packet param
 * @param[out]    buf     Point to paket buf
 * @param[in]     size    Size of packet buf
 *
 * @attention     None
 *
 * @return        None
 */
void aws_build_response(aws_resp_param_t *param, void *buf, uint32_t size);

/**
 * @brief         AWS build notification packet
 *
 * @param[in]     param   Pointer to packet param
 * @param[out]    buf     Point to paket buf
 * @param[in]     size    Size of packet buf
 *
 * @attention     None
 *
 * @return        None
 */
void aws_build_notification(aws_noti_param_t *param, void *buf, uint32_t size);

#endif // __AWS_BUILDER_H

/* End of file -------------------------------------------------------- */
