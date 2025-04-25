#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "HQCoreS3.h"

void bsp_btn_init(void)
{
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = 1ULL << BSP_BUTTOM_GPIO_NUM;
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_config(&io_conf);
}

bool bsp_btn_get(void)
{
  static TickType_t last_tick = 0;
  if(xTaskGetTickCount() - last_tick < pdMS_TO_TICKS(200)) {
    return false;
  } else {
    last_tick = xTaskGetTickCount();
    return gpio_get_level(BSP_BUTTOM_GPIO_NUM) == 0;
  }
}