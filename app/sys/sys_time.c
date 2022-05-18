/**
 * @file       sys_time.c
 * @copyright  Copyright (C) 2020 Hydratech. All rights reserved.
 * @license    This project is released under the Hydratech License.
 * @version    1.0.0
 * @date       2022-03-03
 * @author     Thuan Le
 * @brief      System file to handle system time
 * @note       None
 * @example    None
 */

/* Includes ----------------------------------------------------------- */
#include "sys_time.h"
#include "esp_sntp.h"
#include "bsp_timer.h"
#include "sys_wifi.h"
#include "sys_nvs.h"
#include "bsp.h"

/* Private defines ---------------------------------------------------- */
#define ONE_MS_SECOND         (1)
#define ONE_SECOND            (1000 * ONE_MS_SECOND)
#define ONE_MINUTE            (60 * ONE_SECOND)
#define ONE_HOUR              (60 * ONE_MINUTE)
#define NTP_SYNC_INTEVAL_MS   (ONE_HOUR)

/* Private enumerate/structure ---------------------------------------- */
/*
NTP server struct
*/
typedef struct
{
  char addr[32]; // "xxx.xxx.xxx.xxx" or DNS
  uint16_t port; // 1 - 65535
}
ntp_server_t;

/* Private macros ----------------------------------------------------- */
/* Public variables --------------------------------------------------- */
/* Private variables -------------------------------------------------- */
static bool m_is_ntp_server_initialize = false;
static auto_timer_t m_ntp_atm;

#define INFO(_i, _addr, _port) [_i] = { .addr = _addr, .port = _port }
static const ntp_server_t NTP_SERVER[] = 
{
  //     +=======+===================+=======+
  //     | Index | Address           | Port  |
  //     +=======+===================+=======+
    INFO ( 0     , "time.google.com" , 123   )
  , INFO ( 1     , "time.windows.com", 123   )
  , INFO ( 2     , "time.apple.com"  , 123   )
  //     +===========================+=======+
};
#undef INFO

static char *TAG = "sys_time";

/* Private function prototypes ---------------------------------------- */
static void m_sys_time_initialize_ntp(void);
static void m_sys_time_sync_notification_cb(struct timeval *tv);
static void m_sys_timer_ntp_callback(void *para);
static void m_sys_time_sync_time_ntp(void);

/* Function definitions ----------------------------------------------- */
void sys_time_init(void)
{
  bsp_tmr_auto_init(&m_ntp_atm, m_sys_timer_ntp_callback);
  bsp_tmr_auto_start(&m_ntp_atm, ONE_SECOND);
}

void sys_time_get_epoch_ms(uint64_t *epoch)
{
  struct timeval tv;

  gettimeofday(&tv, NULL);
  *epoch = (tv.tv_sec * 1000LL + (tv.tv_usec / 1000LL));
}

/* Private function definitions --------------------------------------------- */
/**
 * @brief Time sync notification callback
 */
static void m_sys_time_sync_notification_cb(struct timeval *tv)
{
  ESP_LOGI(TAG, "Notification of a time synchronization event");
}

/**
 * @brief Initialize NTP
 */
static void m_sys_time_initialize_ntp(void)
{
  sntp_setoperatingmode(SNTP_OPMODE_POLL);

  sntp_setservername(0, (char *)NTP_SERVER[0].addr);
  sntp_setservername(1, (char *)NTP_SERVER[1].addr);
  sntp_setservername(2, (char *)NTP_SERVER[2].addr);

  sntp_set_time_sync_notification_cb(m_sys_time_sync_notification_cb);
  sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
  sntp_init();
  m_is_ntp_server_initialize = true;
}

static void m_sys_timer_ntp_callback(void *para)
{
  ESP_LOGI(TAG, "NTP timer callback");
  
  if (sys_wifi_is_connected())
    m_sys_time_sync_time_ntp();

  bsp_tmr_auto_start(&m_ntp_atm, NTP_SYNC_INTEVAL_MS);
}

static void m_sys_time_sync_time_ntp(void)
{
  #define RETRY_CNT (5)

  int    retry       = 0;
  time_t now         = 0;
  struct tm timeinfo = {0};
  char strftime_buf[64];

  if (!m_is_ntp_server_initialize)
    m_sys_time_initialize_ntp();

  while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < RETRY_CNT)
  {
    ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, RETRY_CNT);
    bsp_delay_ms(2000);
  }
  time(&now);
  localtime_r(&now, &timeinfo);

  strftime(strftime_buf, sizeof(strftime_buf), "%Y/%m/%d,%H:%M:%S,GMT", &timeinfo);
  ESP_LOGI(TAG, "The current date/time is: %s", strftime_buf);

  #undef RETRY_CNT
}

/* End of file -------------------------------------------------------- */
