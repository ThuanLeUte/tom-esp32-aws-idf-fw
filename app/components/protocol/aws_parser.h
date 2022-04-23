/**
* @file       aws_parser.h
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

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __AWS_PARSER_H
#define __AWS_PARSER_H

/* Includes ----------------------------------------------------------- */
#include "platform_common.h"
#include "sys_aws_shadow.h"

/* Public defines ----------------------------------------------------- */
/* Public enumerate/structure ----------------------------------------- */
/**
 * @brief AWS notification param
 */
typedef struct
{
  char patient_id[50];
  uint16_t time[50];
  uint8_t day[50];
  uint8_t question[50];
}
aws_schedule_t;

/* Public macros ------------------------------------------------------ */
/* Public variables --------------------------------------------------- */
/* Public function prototypes ----------------------------------------- */
/**
 * @brief         AWS build packet
 *
 * @param[in]     name     Shadown name
 * @param[in]     buf      Point to paket buf
 * @param[in]     buf_len  Size of packet buf
 * @param[out]    p_data   Pointer to output data
 *
 * @attention     None
 *
 * @return        None
 */
bool aws_parse_shadow_packet(sys_aws_shadow_name_t name, const void *buf, uint16_t buf_len, void *p_data);

#endif // __AWS_PARSER_H

/* End of file -------------------------------------------------------- */
