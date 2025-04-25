#pragma once

#include "stdbool.h"
#include "driver/i2c.h"
#include "driver/spi_common.h"
#include "esp_camera.h"
#include "esp_lcd_touch_cst816s.h"
#include "mpu6050.h"

// Buttom 管脚
#define BSP_BUTTOM_GPIO_NUM        GPIO_NUM_0
// I2C0 BUS 参数
#define BSP_I2C0_DEVICE_COUNT      (2U)
#define BSP_I2C0_TANS_QUE_DEPTH    (BSP_I2C0_DEVICE_COUNT * 2U)
#define BSP_I2C0_SDA_GPIO_NUM      GPIO_NUM_4
#define BSP_I2C0_SCL_GPIO_NUM      GPIO_NUM_5
#define BSP_I2C0_FREQ_HZ           100000 // 100KHz
// I2C0 设备地址
#define BSP_TOUCH_I2C_ADDR         ESP_LCD_TOUCH_IO_I2C_CST816S_ADDRESS // esp_lcd_touch_cst816s.h
#define BSP_MPU6050_I2C_ADDR       MPU6050_I2C_ADDRESS                  // mpu6050.h
// LCD 功能管脚参数
#define BSP_LCD_BK_LIGHT_ON_LEVEL  1
#define BSP_LCD_BK_LIGHT_OFF_LEVEL !BSP_LCD_BK_LIGHT_ON_LEVEL
#define BSP_LCD_BK_LIGHT_GPIO_NUM  GPIO_NUM_48
#define BSP_LCD_TOUCH_CS_GPIO_NUM  GPIO_NUM_NC
// LCD SPI参数
#define BSP_LCD_SPI_HOST           SPI2_HOST
#define BSP_LCD_SPI_FREQ_HZ        (60 * 1000 * 1000) // 60MHz
#define BSP_LCD_SPI_SCLK_GPIO_NUM  GPIO_NUM_21
#define BSP_LCD_SPI_MOSI_GPIO_NUM  GPIO_NUM_47
#define BSP_LCD_SPI_MISO_GPIO_NUM  GPIO_NUM_NC
#define BSP_LCD_SPI_DC_GPIO_NUM    GPIO_NUM_45
#define BSP_LCD_SPI_RST_GPIO_NUM   GPIO_NUM_NC
#define BSP_LCD_SPI_CS_GPIO_NUM    GPIO_NUM_14
#define BSP_LCD_SPI_CMD_BITS       8U
#define BSP_LCD_SPI_PARAM_BITS     8U
// LCD分辨率
#define BSP_LCD_HOR_RES            240U
#define BSP_LCD_VER_RES            280U
// LVGL参数
#define BSP_LVGL_BUFSIZ            (BSP_LCD_HOR_RES * 40) // 每次传输40行像素数据
#define BSP_LVGL_TICK_PERIOD_MS    portTICK_PERIOD_MS
#define BSP_LVGL_TASK_MAX_DELAY_MS 500U
#define BSP_LVGL_TASK_MIN_DELAY_MS 20U
#define BSP_LVGL_TASK_STACK_SIZE   (4 * 1024) // 4KB LVGL obj stack
// #define BSP_LVGL_TASK_PRIORITY     10
#define BSP_LVGL_TASK_MAX_DELAY_MS 500U
#define BSP_LVGL_TASK_MIN_DELAY_MS 20U
// Camera参数
#define BSP_CAM_MODULE_NAME        "HQYJ-S3-CAM"
#define BSP_CAM_PIN_PWDN           GPIO_NUM_NC
#define BSP_CAM_PIN_RESET          GPIO_NUM_NC
#define BSP_CAM_PIN_XCLK           GPIO_NUM_15
// #define BSP_CAM_PIN_SIOD           GPIO_NUM_4 
// #define BSP_CAM_PIN_SIOC           GPIO_NUM_5
#define BSP_CAM_PIN_SIOD           GPIO_NUM_NC
#define BSP_CAM_PIN_SIOC           GPIO_NUM_NC
#define BSP_CAM_PIN_D7             GPIO_NUM_16
#define BSP_CAM_PIN_D6             GPIO_NUM_17
#define BSP_CAM_PIN_D5             GPIO_NUM_18
#define BSP_CAM_PIN_D4             GPIO_NUM_12
#define BSP_CAM_PIN_D3             GPIO_NUM_10
#define BSP_CAM_PIN_D2             GPIO_NUM_8
#define BSP_CAM_PIN_D1             GPIO_NUM_9
#define BSP_CAM_PIN_D0             GPIO_NUM_11
#define BSP_CAM_PIN_VSYNC          GPIO_NUM_6
#define BSP_CAM_PIN_HREF           GPIO_NUM_7
#define BSP_CAM_PIN_PCLK           GPIO_NUM_13
#define BSP_CAM_XCLK_FREQ_HZ       10000000 // 10MHz
// Camera默认配置
#define BSP_CAM_DEFAULT_CONFIG() {			 \
 .pin_pwdn     = BSP_CAM_PIN_PWDN,       \
 .pin_reset    = BSP_CAM_PIN_RESET,      \
 .pin_xclk     = BSP_CAM_PIN_XCLK,       \
 .pin_sccb_sda = BSP_CAM_PIN_SIOD,       \
 .pin_sccb_scl = BSP_CAM_PIN_SIOC,       \
 .pin_d7       = BSP_CAM_PIN_D7,         \
 .pin_d6       = BSP_CAM_PIN_D6,         \
 .pin_d5       = BSP_CAM_PIN_D5,         \
 .pin_d4       = BSP_CAM_PIN_D4,         \
 .pin_d3       = BSP_CAM_PIN_D3,         \
 .pin_d2       = BSP_CAM_PIN_D2,         \
 .pin_d1       = BSP_CAM_PIN_D1,         \
 .pin_d0       = BSP_CAM_PIN_D0,         \
 .pin_vsync    = BSP_CAM_PIN_VSYNC,      \
 .pin_href     = BSP_CAM_PIN_HREF,       \
 .pin_pclk     = BSP_CAM_PIN_PCLK,       \
 .xclk_freq_hz = BSP_CAM_XCLK_FREQ_HZ,   \
 .ledc_timer   = LEDC_TIMER_0,           \
 .ledc_channel = LEDC_CHANNEL_0,         \
 .pixel_format = PIXFORMAT_RGB565,       \
 .frame_size   = FRAMESIZE_240X240,      \
 .jpeg_quality = 12,                     \ 
 .fb_count     = 2,                      \
 .fb_location  = CAMERA_FB_IN_PSRAM,     \
 .grab_mode    = CAMERA_GRAB_WHEN_EMPTY, \
}
                      
// SD卡挂载点
#define BSP_SDCARD_MOUNT_POINT "/sdcard"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief LED初始化
 */
void bsp_led_init();

/**
 * @brief LED彩虹效果
 * @param arg 任务参数
 * @note 该任务需要自行创建
 */
void ostk_led_rinbow(void* arg);

/**
 * @brief 按钮初始化
 */
void bsp_btn_init(void);

/**
 * @brief 获取按钮状态
 * @return true 按下 false 未按下
 */
bool bsp_btn_get(void);

/**
 * @brief I2C初始化
 * @note !!!!!!!!!!!!! 在初始化LVGL和MPU6050之前必须调用该函数来统一配置I2C0总线上的设备 !!!!!!!!!!!!!
 */
void bsp_i2c_init(void);

/**
 * @brief LCD和LVGL初始化
 */
void bsp_lvgl_init(void);

/**
 * @brief LVGL任务函数
 * @param arg 任务参数
 * @note 该任务需要自行创建
 */
void ostk_bsp_lvgl(void *arg);

/**
 * @brief 锁定LVGL
 * @param timeout_ms 超时时间
 * @return true 成功 false 失败
 * @note 递归锁, 该函数会锁定LVGL，防止多任务同时访问
 */
bool bsp_lvgl_lock(int timeout_ms);

/**
 * @brief 解锁LVGL
 */
void bsp_lvgl_unlock(void);

/**
 * @brief MPU6050初始化
 * @note 数据读取直接调用 "mpu6050.h" 中的函数
 */
void bsp_mpu6050_init(void);

/**
 * @brief 摄像头初始化
 * @param ccfg_cam 摄像头配置, 参考 BSP_CAM_DEFAULT_CONFIG()
 * @return 0 成功
 */
int bsp_cam_init(const camera_config_t* ccfg_cam);

/**
 * @brief SD卡初始化
 */
esp_err_t bsp_sdcard_init();

/**
 * @brief SD卡测试函数
 * @note 在SD卡根目录创建hello.txt文件
 */
void bsp_sdcard_hello();

#ifdef __cplusplus
}
#endif