#ifndef __HAL_CONFIG_H
#define __HAL_CONFIG_H

/* GPIO Config*/
#define HAL_GPIO_ENABLED 1

/* TIM Config */
#define HAL_TIM_ENABLE 0

/* UART Config*/
#define HAL_UART_ENABLED 1
#define HAL_LOG_ENABLED 1
#define HAL_CMD_ENABLED 1
#define HAL_SBUS_ENABLED 0
#define HAL_FEETECH_ENABLED 0

/* UART Hardware Channels */
#define HW_UART_CHANNEL_1_ENABLED 0 // UART1 通道未启用
#define HW_UART_CHANNEL_2_ENABLED 0 // UART2 通道未启用
#define HW_UART_CHANNEL_3_ENABLED 1 // UART3 通道未启用
#define HW_UART_CHANNEL_4_ENABLED 0 // UART4 通道启用
#define HW_UART_CHANNEL_5_ENABLED 0 // UART5
#define HW_UART_CHANNEL_6_ENABLED 0 // UART6 通道启用

// 根据硬件配置定义可用UART通道数
#define HW_UART_CHANNEL_COUNT                                                  \
  (HW_UART_CHANNEL_1_ENABLED + HW_UART_CHANNEL_2_ENABLED +                     \
   HW_UART_CHANNEL_3_ENABLED + HW_UART_CHANNEL_4_ENABLED +                     \
   HW_UART_CHANNEL_5_ENABLED + HW_UART_CHANNEL_6_ENABLED)

#define HW_LOG_HUART UART_CHANNEL_3
#define HW_CMD_HUART UART_CHANNEL_3
#define HW_SBUS_HUART UART_CHANNEL_4

#define HW_JODELL_HUART UART_CHANNEL_6

/* SYS Config */
#define HAL_DELAY_ENABLED 1

/* CAN Config*/
#define HAL_CAN_ENABLED 0
#define HW_CAN_CHANNEL_1_ENABLED 1 // CAN1 通道启用
#define HW_CAN_CHANNEL_2_ENABLED 0 // CAN2 通道启用（如果硬件不支持可设为0）

// 根据硬件配置定义可用CAN通道数
#define HW_CAN_CHANNEL_COUNT                                                   \
  (HW_CAN_CHANNEL_1_ENABLED + HW_CAN_CHANNEL_2_ENABLED)

#endif // __HAL_CONFIG_H