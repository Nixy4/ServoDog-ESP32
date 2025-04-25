#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_timer.h"

#define TIMER_PERIOD_US 5000000 // 定义定时器周期为5秒
static esp_timer_handle_t timer_handle;

void print_ram_info(void* arg) {
	// 查看堆总体情况
	size_t iram = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
	size_t iram_min = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);

	size_t dmaram = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
	size_t dmaram_min = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);

	size_t spiram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
	size_t spiram_min = heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM);

	// 打印
  printf("%-10s%-10s%-10s\n", "Type", "Free", "MinFree");
  printf("%-10s%-10d%-10d\n", "IRAM", iram, iram_min);
  printf("%-10s%-10d%-10d\n", "DMARAM", dmaram, dmaram_min);
  printf("%-10s%-10d%-10d\n", "SPIRAM", spiram, spiram_min);
}

void bsp_ram_monitor_enable()
{
  // 创建定时器
	const esp_timer_create_args_t timer_args = {
		.callback = &print_ram_info,
		.name = "ram monitor"
	};
	esp_timer_create(&timer_args, &timer_handle);
	esp_timer_start_periodic(timer_handle, TIMER_PERIOD_US);
}

void bsp_ram_monitor_disable()
{ 
  // 停止定时器
  esp_timer_stop(timer_handle);
  // 删除定时器
  esp_timer_delete(timer_handle);
}