#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/sdmmc_host.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "HQCoreS3.h"

static const char* TAG = "BSP-SDCARD";

// SemaphoreHandle_t gpio38_mutex = NULL;

// bool bsp_sdcard_lock(int timeout_ms)
// {
// 	timeout_ms = timeout_ms < 0 ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
// 	return xSemaphoreTakeRecursive(gpio38_mutex, timeout_ms) == pdTRUE;
// }

// void bsp_sdcard_unlock()
// {
// 	xSemaphoreGiveRecursive(gpio38_mutex);
// }

esp_err_t bsp_sdcard_init()
{
	// gpio38_mutex = xSemaphoreCreateRecursiveMutex();
	// bsp_sdcard_lock(-1);//!

	ESP_LOGI(TAG, "Initializing SD card, Using SDMMC peripheral");
	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
		.format_if_mount_failed = false,
		.max_files = 5,
		.allocation_unit_size = 16 * 1024
	};
	sdmmc_card_t *card;
	sdmmc_host_t host = SDMMC_HOST_DEFAULT();
	sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
	slot_config.width = 1;
	slot_config.clk = GPIO_NUM_39;
	slot_config.cmd = GPIO_NUM_38;
	slot_config.d0 = GPIO_NUM_40;
	slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
	esp_err_t err = esp_vfs_fat_sdmmc_mount(BSP_SDCARD_MOUNT_POINT, &host, &slot_config, &mount_config, &card);

	if (err != ESP_OK) 
	{
		if (err == ESP_FAIL) {
			ESP_LOGE(TAG, 
				"挂载文件系统失败, 如果您希望格式化卡,请设置EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig选项");
		} else {
			ESP_LOGE(TAG,
				"初始化卡失败(%s), 确保SD卡线路上有上拉电阻。", esp_err_to_name(err));
		}
		return err;
	}
	ESP_LOGI(TAG, "文件系统已挂载");
	sdmmc_card_print_info(stdout, card);

	// bsp_sdcard_unlock();//!
	return ESP_OK;
}

void bsp_sdcard_hello()
{
	// 首先创建一个文件
	const char *file_hello = BSP_SDCARD_MOUNT_POINT"/hello.txt";
	ESP_LOGI(TAG, "打开文件 %s", file_hello);
	FILE *f = fopen(file_hello, "w");
	if (f == NULL) {
			ESP_LOGE(TAG, "打开文件写入失败");
			return;
	}
	fprintf(f, "Hello %s!\n", "sdcard");
	fclose(f);
	ESP_LOGI(TAG, "文件已写入");
}