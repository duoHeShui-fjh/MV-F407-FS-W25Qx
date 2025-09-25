#include "app.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"


#include "driver_cmd.h"
#include "driver_json.h"
#include "driver_log.h"

#include "hal_gpio.h"
#include "hal_delay.h"
#include "stm32f4xx_hal_def.h"

#include <stdint.h>
#include <string.h>

#include "sfud.h"
#include "driver_fs.h"

static const hal_delay_interface_t *hal_delay = NULL;
static const hal_gpio_interface_t *hal_gpio = NULL;

static const driver_cmd_interface_t *cmd = NULL;
static const driver_log_interface_t *logger = NULL;

static const driver_json_interface_t *json = NULL;

void app_sys_init(void) {
  // 抽象层接口获取
  hal_delay = hal_delay_get_interface();
  hal_gpio = hal_gpio_get_interface();

  // 驱动层接口获取
  cmd = driver_cmd_get_interface();
  logger = driver_log_get_interface();

  json = driver_json_get_interface();

  // 驱动层初始化
  cmd->init();
  logger->init();

  json->init();
}

void app_default_task(void) { 
  /* USER CODE BEGIN StartDefaultTask */
  sfud_init(); // W25Qxx通用接口初始化
  int cnt = 0;

  // // 安全初始化文件系统 - 保护现有数据
  // safe_init_filesystem(0);
  safe_init_filesystem(1);

  // // 显示文件系统信息和目录结构
  show_partition_info();
  show_directory_tree(NULL);
  show_all_file_contents(NULL);

  // // 演示文件系统功能
  demo_filesystem();

  // // // 显示分区信息和所有路径
  // osDelay(100);
  show_partition_info();
  show_directory_tree(NULL);
  show_all_file_contents(NULL);
  /* Infinite loop */
  for (;;) {
    // printf("Hello World! %d\n", cnt);
    cnt++;
    osDelay(2000);
  } 
}

void app_led_r_task(void) {
  for (;;) {
    hal_gpio->toggle(LED_RED_GPIO_Port, LED_RED_Pin);
    hal_delay->delay_ms(100);
  }
}

void app_led_b_task(void) {
  for (;;) {
    hal_gpio->toggle(LED_BLUE_GPIO_Port, LED_BLUE_Pin);
    hal_delay->delay_ms(100);
  }
}

void app_log_task(void) {
  if (!logger) {
    return;
  }
  for (;;) {
    logger->process_queue();
  }
}


void app_cmd_task(void) {
  if (!cmd) {
    log_warn("cmd driver is null, skipping!");
    return;
  }
  for (;;) {
    cmd->process_queue();
  }
}
