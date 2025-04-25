#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"

#include "nvs_flash.h"
#include "lvgl.h"

#include "HQCoreS3.h"

#ifdef __cplusplus
extern "C" {
#endif

void app_main(void)
{
	//! NVS  非易失存储器
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
			ESP_ERROR_CHECK(nvs_flash_erase());
			ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret); 

	//! 先做硬件初始化
	bsp_led_init();
	bsp_btn_init();
	bsp_i2c_init();
	const camera_config_t cam_cfg = BSP_CAM_DEFAULT_CONFIG();
	bsp_cam_init(&cam_cfg);
	bsp_lvgl_init();

	//! 再开始统一调度任务
	xTaskCreatePinnedToCore(ostk_led_rinbow, "LED-RINBOW", 4096, NULL, 5, NULL, APP_CPU_NUM);
	//*参数1:任务函数指针(任务函数)
	//参数2:任务名称(字符串), 在操作系统的调试中功能中可以打印出来看
	//*参数3:任务栈大小(字节), 4096字节, 我们的板子使用 8MB的片外内存
	//参数4:任务参数(指针), 传递给任务函数的参数, 这里我们不需要, 所以传NULL
	//*参数5:任务优先级, 0-15, 0最低, 15最高, 我们这套驱动不要低于5
	//参数6:任务句柄(指针), 传NULL, 我们不需要
	//*参数7:任务运行的CPU
	//			PRO_CPU_NUM : 协议CPU, 主要运行WiFi,蓝牙,以及其他通信协议相关的程序
	//     	APP_CPU_NUM : 应用CPU, 主要运行应用程序, 也就是我们写的代码
	xTaskCreatePinnedToCore(ostk_bsp_lvgl,  "BSP-LVGL",    4096, NULL, 5, NULL, APP_CPU_NUM);
	
	//! 绘制一点简单的图像
	bsp_lvgl_lock(1000);//! 等待LVGL锁定成功
	lv_obj_t * scr = lv_scr_act(); // 获取当前屏幕对象
	lv_obj_t * label = lv_label_create(scr);
	lv_label_set_text(label, "Hello World!"); // 设置标签文本
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); // 设置标签位置
	lv_obj_set_style_text_color(label, lv_color_hex(0xFF0000), 0); // 设置标签颜色
	lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0); // 设置标签字体
	bsp_lvgl_unlock();//! 释放LVGL锁定

	return;
}

#ifdef __cplusplus
}
#endif
