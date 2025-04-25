#include <stdio.h>
#include "driver/i2c.h"
#include "esp_system.h"
#include "esp_log.h"
#include "mpu6050.h"

static mpu6050_handle_t mpu6050 = NULL;

void bsp_mpu6050_init(void)
{
  esp_err_t ret;
  mpu6050 = mpu6050_create(I2C_NUM_0, MPU6050_I2C_ADDRESS);
  ret = mpu6050_config(mpu6050, ACCE_FS_4G, GYRO_FS_500DPS);
  ret = mpu6050_wake_up(mpu6050);
}
