#ifndef __APP_H
#define __APP_H

void app_sys_init(void);

void app_default_task(void);

void app_led_r_task(void);
void app_led_b_task(void);

void app_log_task(void);

void app_cmd_task(void);



#endif // __APP_H