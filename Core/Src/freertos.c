/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "diskio.h"
#include "fatfs.h"
#include "ffconf.h"
#include "gpio.h"
#include "sfud.h"
#include <stdint.h>
#include <stdio.h>

#include "driver_flash.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for LED_R_TASK */
osThreadId_t LED_R_TASKHandle;
const osThreadAttr_t LED_R_TASK_attributes = {
  .name = "LED_R_TASK",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for LED_B_TASK */
osThreadId_t LED_B_TASKHandle;
const osThreadAttr_t LED_B_TASK_attributes = {
  .name = "LED_B_TASK",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void os_led_r_task(void *argument);
void os_led_b_task(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of LED_R_TASK */
  LED_R_TASKHandle = osThreadNew(os_led_r_task, NULL, &LED_R_TASK_attributes);

  /* creation of LED_B_TASK */
  LED_B_TASKHandle = osThreadNew(os_led_b_task, NULL, &LED_B_TASK_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param    printf("Initializing filesystem...\n");
  init_filesystem();
argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
  printf("Starting...v 0.1.0\n");
  sfud_init();
  int cnt = 0;

  // SFUD测试
  /*
  const driver_flash_interface_t *flash = driver_flash_get_interface();

  uint8_t data[100] = {0};
  uint32_t data_len = sizeof(data);

  flash->read(0, data_len, data);
  for (uint8_t i = 0; i < data_len; i++) {
    printf("%d ", data[i]);
    if (i % 10 == 9) {
      printf("\n");
    }
  }
  printf("\n");

  osDelay(1000);
  for (uint32_t i = 0; i < data_len; i++)
    data[i] = i;
  flash->write(0, data_len, data);

  osDelay(1000);
  flash->read(0, data_len, data);
  for (uint8_t i = 0; i < data_len; i++) {
    printf("%d ", data[i]);
    if (i % 10 == 9) {
      printf("\n");
    }
  }
  printf("\n");

  osDelay(1000);
  flash->erase(0);
*/

  // 安全初始化文件系统 - 保护现有数据
  safe_init_filesystem();

  // 创建文件夹（如果不存在的话）
  create_directory("bin");
  create_directory("lib");

  // 创建文件
  char file_name[20] = "bin/test4.txt";
  char file_content[100] = "Hello World!\r\nhello fatfs1\r\n";
  create_file(file_name, file_content);
  char file_name2[20] = "bin/test5.txt";
  char file_content2[100] = "Hello World!\r\nhello fatfs3\r\n";
  create_file(file_name2, file_content2);
  char file_name3[20] = "lib/test6.txt";
  char file_content3[100] = "Hello World!\r\nhello fatfs3\r\n";
  create_file(file_name3, file_content3);

  char read_content[100] = {0};
  read_file("bin/test1.txt", read_content);

  list_files(USERPath);

  // 显示文件系统信息和目录结构
  show_filesystem_info();

  // 显示分区信息和所有路径
  show_partition_info();
  /* Infinite loop */
  for (;;) {
    // printf("Hello World! %d\n", cnt);
    cnt++;
    osDelay(2000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_os_led_r_task */
/**
 * @brief Function implementing the LED_R_TASK thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_os_led_r_task */
void os_led_r_task(void *argument)
{
  /* USER CODE BEGIN os_led_r_task */
  /* Infinite loop */
  for (;;) {
    HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin);
    osDelay(100);
  }
  /* USER CODE END os_led_r_task */
}

/* USER CODE BEGIN Header_os_led_b_task */
/**
 * @brief Function implementing the LED_B_TASK thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_os_led_b_task */
void os_led_b_task(void *argument)
{
  /* USER CODE BEGIN os_led_b_task */
  /* Infinite loop */
  for (;;) {
    HAL_GPIO_TogglePin(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
    osDelay(100);
  }
  /* USER CODE END os_led_b_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

