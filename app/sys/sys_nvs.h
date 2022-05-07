/**
 * @file       sys_nvs.h
 * @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
 * @license    This project is released under the Hydratech License.
 * @version    1.0.0
 * @date       2022-02-15
 * @author     Thuan Le
 * @brief      NVS flash memory handle
 * @note       None
 * @example    None
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __SYS_NVS_H
#define __SYS_NVS_H

/* Includes ----------------------------------------------------------- */
#include "platform_common.h"

/* Public defines ----------------------------------------------------- */
// IMPORTANT: The revision of nvs_data_t. Everytime nvs_data_t is changed, 
// the NVS_DATA_VERSION value must be updated too.
#define NVS_DATA_VERSION    (uint32_t)(0x000000A2)

/* Public enumerate/structure ----------------------------------------- */
typedef struct nvs_data_struct
{
  uint32_t data_version;      // Version of NVS data

  struct
  {
    char qr_code[50];      // QR code
    uint8_t qr_code_flag;  // Flag indicate that QR code is set or not
  } dev;

  char thing_name[50];
  char mac_device_addr[32];
  uint8_t provision_status;

  struct
  {
    uint8_t status;
    bool enable;
    char url[100];
  }
  ota;

  uint16_t scale_tare;
}
nvs_data_t;

/* Public macros ------------------------------------------------------ */
#define SYS_NVS_LOOKUP_KEY(name)                                  \
  sys_nvs_lookup_key(offsetof(struct nvs_data_struct, name)       \
                     , sizeof(g_nvs_setting_data.name))           \

/**
 * @brief  Store one specific data belongs to nvs_data_struct into NVS storage
 *
 * @param[in]  member  the name of data in the structure @ref nvs_data_struct
 *                     Example: SYS_NVS_STORE(sample1); 
 *                           or SYS_NVS_STORE(sample2); 
 *
 *
 */
#define SYS_NVS_STORE(member)                                     \
  do                                                              \
  {                                                               \
    sys_nvs_store(SYS_NVS_LOOKUP_KEY(member),                     \
                  &g_nvs_setting_data.member,                     \
                  sizeof(g_nvs_setting_data.member));             \
  }                                                               \
  while (0)                                                       \

/**
 * @brief  load one specific data from NVS storage to nvs_data_struct.
 *
 * @param[in]  member  the name of data in the structure @ref nvs_data_struct
 *                     Example: SYS_NVS_LOAD(sample1); 
 *                           or SYS_NVS_LOAD(sample2); 
 *
 *
 */
#define SYS_NVS_LOAD(member)                                      \
  do                                                              \
  {                                                               \
    sys_nvs_load(SYS_NVS_LOOKUP_KEY(member),                      \
                  &g_nvs_setting_data.member,                     \
                  sizeof(g_nvs_setting_data.member));             \
  }                                                               \
  while (0)                                                       \

/* Public variables --------------------------------------------------- */
extern nvs_data_t g_nvs_setting_data;

/* Public function prototypes ----------------------------------------- */
/**
 * @brief  Init NVS storage and automatically load data to RAM if the data version is valid.
 *         In case of data version is different, all data will be set to default value both in NVS and RAM.
 *
 * @return  None
 */
void sys_nvs_init(void);

/**
 * @brief  Deinit the NVS storage by closing it. 
 *
 * @return  None
 */
void sys_nvs_deinit(void);

/**
 * @brief  Immediately store all data from @ref g_nvs_setting_data structure into NVS storage.
 *
 * @return  None
 */
void sys_nvs_store_all(void);

/**
 * @brief  Immediately load all data from NVS storage to  @ref g_nvs_setting_data structure
 *
 * @return  None
 */
void sys_nvs_load_all(void);

/**
 * @brief  Erase all data in NVS storage.
 *
 * @return  None
 */
void sys_nvs_factory_reset(void);

/**
 * @brief  Store one specific data into NVS storage
 *
 * @param[in]     p_key_name  pointer to the key name to pair with data, limit under 16 character.
 * @param[in]     p_src       pointer to buffer contains data.
 * @param[in]     len         length of data in bytes.
 *
 * @return  None
 */
void sys_nvs_store(char *p_key_name, void *p_src, uint32_t len);

/**
 * @brief  Load one specific data from NVS storage to destination buffer.
 *
 * @param[in]     p_key_name  pointer to the key name to pair with data, limit under 16 character.
 * @param[out]    p_des       pointer to buffer will contain data.
 * @param[in]     len         length of the buffer in bytes.
 *
 * @return  None
 */
void sys_nvs_load(char *p_key_name, void *p_des, uint32_t len);

/**
 * @brief  Automatically look up the ID entry based in input parameter: offset and size of variable
 *
 * @param[in]     offset  offset 
 * @param[in]     size    size of data in bytes.
 *
 * @attention the look up table is @ref VAR_DATA_ID_LIST[]
 * 
 * @return  pointer to key name
 */
char *sys_nvs_lookup_key(uint32_t offset, uint32_t size);

#endif // __SYS_NVS_H

/* End of file -------------------------------------------------------- */
