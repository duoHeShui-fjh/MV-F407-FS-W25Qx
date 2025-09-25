#ifndef __DEVICE_CONFIG_H
#define __DEVICE_CONFIG_H

#include "main.h"

#define DRIVER_LED_ENABLED 0
#define DRIVER_RGB_ENABLED 0
#define DRIVER_BRT_ENABLED 0
#define DRIVER_ZDT_ENABLED 0
#define DRIVER_ZDT_EMM_ENABLED 0
#define DRIVER_JODELL_ENABLED 0

#define DRIVER_DM_ENABLED 0

// 这里是设备驱动的配置选项
// LED配置 - 复用main.h中的定义
#define HW_LED_PORT LED_GPIO_Port
#define HW_LED_PIN LED_Pin
#define HW_LED_ACTIVE_LOW 0

#endif // __DEVICE_CONFIG_H