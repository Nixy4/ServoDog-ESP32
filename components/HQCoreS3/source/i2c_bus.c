#include <stdio.h>
#include "esp_system.h"
#include "esp_log.h"
#include "mpu6050.h"
#include "HQCoreS3.h"

#if ESP_IDF_VERSION_MAJOR>=5 && ESP_IDF_VERSION_MINOR<=3

#include "driver/i2c.h"

void bsp_i2c_init(void)
{
  // idf 5.3.2 版本配置 老版本I2C驱动 driver/i2c.h
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = BSP_I2C0_SDA_GPIO_NUM;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = BSP_I2C0_SCL_GPIO_NUM;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = BSP_I2C0_FREQ_HZ;
  conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;
  esp_err_t ret = i2c_param_config(I2C_NUM_0, &conf);
  ret = i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
}

#elif ESP_IDF_VERSION_MAJOR>=5 && ESP_IDF_VERSION_MINOR>=4

#include "driver/i2c_master.h"

void bsp_i2c_init(void)
{
  // idf 5.4 版本配置 新版本I2C驱动 driver-ng /driver/i2c_master.h
  // 添加总线-I2C0
  const i2c_master_bus_config_t conf = {
    .i2c_port = I2C_NUM_0,
    .sda_io_num = BSP_I2C0_SDA_GPIO_NUM,
    .scl_io_num = BSP_I2C0_SCL_GPIO_NUM,
    .clk_source = I2C_CLK_SRC_XTAL,
    .glitch_ignore_cnt = 7,
    .intr_priority = 0,
    .trans_queue_depth = BSP_I2C0_TANS_QUE_DEPTH,
    .flags = {
      .enable_internal_pullup = 1,
    },
  };
  i2c_master_bus_handle_t i2c_handle = NULL;
  i2c_new_master_bus(&conf,&i2c_handle);
  // 添加设备-触摸屏
  i2c_device_config_t touch_conf = {
    .dev_addr_length = I2C_ADDR_BIT_7,
    .device_address = BSP_TOUCH_I2C_ADDR,
    .scl_speed_hz = 100000,
    .scl_wait_us = 0,
    .flags = {
      .disable_ack_check = 0,
    },
  };
  i2c_master_dev_handle_t touch_handle = NULL;
  i2c_master_bus_add_device(i2c_handle, &touch_conf, &touch_handle);
  // 添加设备-MPU6050
  i2c_device_config_t mpu_conf = {
    .dev_addr_length = I2C_ADDR_BIT_7,
    .device_address = BSP_MPU6050_I2C_ADDR,
    .scl_speed_hz = 100000,
    .scl_wait_us = 0,
    .flags = {
      .disable_ack_check = 0,
    },
  };
  i2c_master_dev_handle_t mpu_handle = NULL;
  i2c_master_bus_add_device(i2c_handle, &mpu_conf, &mpu_handle);
}

#endif

