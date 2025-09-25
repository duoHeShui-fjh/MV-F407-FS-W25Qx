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
static const driver_fs_interface_t *fs = NULL;

static const driver_json_interface_t *json = NULL;

void app_sys_init(void) {
  // 抽象层接口获取
  hal_delay = hal_delay_get_interface();
  hal_gpio = hal_gpio_get_interface();

  // 驱动层接口获取
  cmd = driver_cmd_get_interface();
  logger = driver_log_get_interface();
  fs = driver_fs_get_interface();

  json = driver_json_get_interface();

  // 驱动层初始化
  cmd->init();
  logger->init();

  json->init();
}

void app_default_task(void) {
  fs->init();
  hal_delay->delay_ms(10);
  fs->init_fs(0);
  // fs->show_all_file_contents(NULL);
  // fs->demo_filesystem();
  // fs->show_directory_tree(NULL);
  // fs->show_all_file_contents(NULL);
  // fs->append_file("/log.txt", "Third line\n");
  // fs->show_partition_info();
  // fs->delete_file("/log.txt");
  // fs->append_file("/log.txt", "Second line0\r\n");
  char read_buf[100] = {0};
  fs->read_file("/log.txt", read_buf);
  hal_delay->delay_ms(10);

  fs->show_directory_tree(NULL);
  fs->show_partition_info();

  hal_delay->delay_ms(10);
  log_info("full cnt:%d", logger->queue_full_count);

  /* Infinite loop */
  for (;;) {
    // static int cnt = 0;
    // printf("Hello World! %d\n", cnt++);
    // log_info("full cnt:%d", logger->queue_full_count);

    hal_delay->delay_ms(50000);
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
