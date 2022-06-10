/**
 * @file       sys_variant.h
 * @note       This is were we put the global defines that are used to
 * condiftional compile the vairants and features in/out
 * @example    None
 */

/* Define to prevent recursive inclusion ------------------------------ */
#ifndef __SYS_VARIANT_H
#define __SYS_VARIANT_H

#define __CONFIG_SOFT_AP_MODE   (1)    //eaable soft AP mode for xx mins after reset, mostly for development
#define __CONFIG_BLE_CONFIG_MODE   (1)    //eaable BLE config mode for xx mins after reset, mostly for myCaire phone app
#define __LOX_BUILD__           (0)     //enable build for LOX product
#define __TPOC_BUILD__           (0)     //enable build for TPOC Concentrator product
#define __GSOC_BUILD__           (0)     //enable build for Global SOC product


#endif /* __SYS_VARIANT_H */