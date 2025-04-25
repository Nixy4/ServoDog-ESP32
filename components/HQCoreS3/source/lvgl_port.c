#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch_cst816s.h"
#include "lvgl.h"
#include "HQCoreS3.h"

static const char* TAG = "BSP-LVGL";

static SemaphoreHandle_t osmtx_lvgl = NULL;
static bool touch_ok = false;
static esp_lcd_touch_handle_t touch_handle = NULL;

esp_err_t touchpad_i2c_check(void)
{
	uint8_t address;
	printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
	for (int i = 0; i < 128; i += 16)
	{
		printf("%02x: ", i);
		for (int j = 0; j < 16; j++)
		{
			fflush(stdout);
			address = i + j;
			i2c_cmd_handle_t cmd = i2c_cmd_link_create();
			i2c_master_start(cmd);
			i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1);
			i2c_master_stop(cmd);
			esp_err_t ret = i2c_master_cmd_begin(0, cmd, 50 / portTICK_PERIOD_MS);
			i2c_cmd_link_delete(cmd);
			if (ret == ESP_OK)
			{
				printf("%02x ", address);
				if (address == 0X15)
				{
					printf("\r\n");
					ESP_LOGI("I2C", "找到设备，地址: 0x%02X, 返回 OK", address);
					touch_ok = true;
					return ESP_OK; // 找到 0x15 地址，返回 OK
				}
			}
			else if (ret == ESP_ERR_TIMEOUT)
			{
				printf("UU ");
			}
			else
			{
				printf("-- ");
			}
		}
		printf("\r\n");
	}
	ESP_LOGE("I2C", "未找到地址 0x15, 返回失败");
	touch_ok = false;
	return ESP_FAIL; // 没有找到设备
}

// void touchpad_i2c_init(void)
// {
// 	const i2c_config_t i2c_conf = {
// 		.mode             = I2C_MODE_MASTER,
// 		.sda_io_num       = 4,
// 		.sda_pullup_en    = GPIO_PULLUP_ENABLE,
// 		.scl_io_num       = 5,
// 		.scl_pullup_en    = GPIO_PULLUP_ENABLE,
// 		.master.clk_speed = 100000
// 		};
// 	i2c_param_config(I2C_NUM_0, &i2c_conf);
// 	i2c_driver_install(I2C_NUM_0, i2c_conf.mode, 0, 0, 0);
// }
// 已经在 bsp_i2c_init() 中初始化

static bool bsp_lv_flush_ready_cb(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
	lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
	lv_disp_flush_ready(disp_driver);
	return false;
}

static void bsp_lv_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
	esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
	int offsetx1 = area->x1;
	int offsetx2 = area->x2;
	int offsety1 = area->y1 + 20;
	int offsety2 = area->y2 + 20;
	esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

static void bsp_lv_port_update_cb(lv_disp_drv_t *drv)
{
	esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;

	switch (drv->rotated)
	{
	case LV_DISP_ROT_NONE:
		// Rotate LCD display
		esp_lcd_panel_swap_xy(panel_handle, false);
		esp_lcd_panel_mirror(panel_handle, true, false);
		// Rotate LCD touch
		esp_lcd_touch_set_mirror_y(touch_handle, false);
		esp_lcd_touch_set_mirror_x(touch_handle, false);
		break;
	case LV_DISP_ROT_90:
		// Rotate LCD display
		esp_lcd_panel_swap_xy(panel_handle, true);
		esp_lcd_panel_mirror(panel_handle, true, true);
		// Rotate LCD touch
		esp_lcd_touch_set_mirror_y(touch_handle, false);
		esp_lcd_touch_set_mirror_x(touch_handle, false);
		break;
	case LV_DISP_ROT_180:
		// Rotate LCD display
		esp_lcd_panel_swap_xy(panel_handle, false);
		esp_lcd_panel_mirror(panel_handle, false, true);
		// Rotate LCD touch
		esp_lcd_touch_set_mirror_y(touch_handle, false);
		esp_lcd_touch_set_mirror_x(touch_handle, false);
		break;
	case LV_DISP_ROT_270:
		// Rotate LCD display
		esp_lcd_panel_swap_xy(panel_handle, true);
		esp_lcd_panel_mirror(panel_handle, false, false);
		// Rotate LCD touch
		esp_lcd_touch_set_mirror_y(touch_handle, false);
		esp_lcd_touch_set_mirror_x(touch_handle, false);
		break;
	}
}

static void bsp_lv_touch_cb(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
	uint16_t esph_touchpad_x[1] = {0};
	uint16_t esph_touchpad_y[1] = {0};
	uint8_t esph_touchpad_cnt = 0;

	/* Read touch controller data */
	esp_lcd_touch_read_data(drv->user_data);

	/* Get coordinates */
	bool esph_touchpad_pressed = esp_lcd_touch_get_coordinates(drv->user_data, esph_touchpad_x, esph_touchpad_y, NULL, &esph_touchpad_cnt, 1);

	if (esph_touchpad_pressed && esph_touchpad_cnt > 0)
	{
		data->point.x = esph_touchpad_x[0];
		data->point.y = esph_touchpad_y[0];
		data->state = LV_INDEV_STATE_PRESSED;
	}
	else
	{
		data->state = LV_INDEV_STATE_RELEASED;
	}
}

static void bsp_lv_tick_inc_cb(void *arg)
{
	/* Tell LVGL how many milliseconds has elapsed */
	lv_tick_inc(BSP_LVGL_TICK_PERIOD_MS);
}

bool bsp_lvgl_lock(int timeout_ms)
{
	// Convert timeout in milliseconds to FreeRTOS ticks
	// If `timeout_ms` is set to -1, the program will block until the condition is met
	const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
	return xSemaphoreTakeRecursive(osmtx_lvgl, timeout_ticks) == pdTRUE;
}

void bsp_lvgl_unlock(void)
{
	xSemaphoreGiveRecursive(osmtx_lvgl);
}

void ostk_bsp_lvgl(void *arg)
{
	uint32_t task_delay_ms = BSP_LVGL_TASK_MAX_DELAY_MS;
	while (1)
	{
		// ESP_LOGW(TAG, "bsp_lv_port_task start");
		if (bsp_lvgl_lock(-1))
		{
			task_delay_ms = lv_timer_handler();
			bsp_lvgl_unlock();
		}
		if (task_delay_ms > BSP_LVGL_TASK_MAX_DELAY_MS)
		{
			task_delay_ms = BSP_LVGL_TASK_MAX_DELAY_MS;
		}
		else if (task_delay_ms < BSP_LVGL_TASK_MIN_DELAY_MS)
		{
			task_delay_ms = BSP_LVGL_TASK_MIN_DELAY_MS;
		}
		vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
		// ESP_LOGW(TAG, "bsp_lv_port_task end");
	}
}

void bsp_lvgl_init(void)
{
	static lv_disp_draw_buf_t disp_buf; // contains internal graphic buffer(s) called draw buffer(s)
	static lv_disp_drv_t disp_drv;      // contains callback functions

	ESP_LOGI(TAG, "Turn off LCD backlight");
	gpio_config_t bk_gpio_config = {
		.mode = GPIO_MODE_OUTPUT,
		.pin_bit_mask = 1ULL << BSP_LCD_BK_LIGHT_GPIO_NUM};
	ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));

	ESP_LOGI(TAG, "Initialize SPI bus");
	spi_bus_config_t buscfg = {
		.sclk_io_num = BSP_LCD_SPI_SCLK_GPIO_NUM,
		.mosi_io_num = BSP_LCD_SPI_MOSI_GPIO_NUM,
		.miso_io_num = BSP_LCD_SPI_MISO_GPIO_NUM,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = BSP_LVGL_BUFSIZ * sizeof(uint16_t),
	};
	ESP_ERROR_CHECK(spi_bus_initialize(BSP_LCD_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

	ESP_LOGI(TAG, "Install panel IO");
	esp_lcd_panel_io_handle_t io_handle = NULL;
	esp_lcd_panel_io_spi_config_t io_config = {
		.dc_gpio_num = BSP_LCD_SPI_DC_GPIO_NUM,
		.cs_gpio_num = BSP_LCD_SPI_CS_GPIO_NUM,
		.pclk_hz = BSP_LCD_SPI_FREQ_HZ,
		.lcd_cmd_bits = BSP_LCD_SPI_CMD_BITS,
		.lcd_param_bits = BSP_LCD_SPI_PARAM_BITS,
		.spi_mode = 0,
		.trans_queue_depth = 10,
		.on_color_trans_done = bsp_lv_flush_ready_cb,
		.user_ctx = &disp_drv,
	};
	// Attach the LCD to the SPI bus
	ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)BSP_LCD_SPI_HOST, &io_config, &io_handle));

	esp_lcd_panel_handle_t panel_handle = NULL;
	esp_lcd_panel_dev_config_t panel_config = {
		.reset_gpio_num = BSP_LCD_SPI_RST_GPIO_NUM,
		.rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
		.bits_per_pixel = 16,
	};

	ESP_LOGI(TAG, "Install ST7789 panel driver");
	ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));

	ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
	ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
	ESP_ERROR_CHECK(esp_lcd_panel_swap_xy(panel_handle, false));
	ESP_ERROR_CHECK(esp_lcd_panel_mirror(panel_handle, false, false));
	ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
	ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

	// touchpad_i2c_init();
	touchpad_i2c_check();

	if (touch_ok == true)
	{
		/************* 初始化触摸屏 **************/
		esp_lcd_panel_io_handle_t esph_touchpad_io_handle = NULL;
		esp_lcd_panel_io_i2c_config_t esph_touchpad_io_config = ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
		esp_lcd_touch_config_t esph_touchpad_cfg = {
			.x_max = BSP_LCD_HOR_RES,
			.y_max = BSP_LCD_VER_RES,
			.rst_gpio_num = -1,
			.int_gpio_num = -1,
			.flags = {
				.swap_xy = 0,
				.mirror_x = 0,
				.mirror_y = 0,
			},
		};
		ESP_LOGI(TAG, "Initialize touch controller cst816s");
		esp_lcd_new_panel_io_i2c((esp_lcd_i2c_bus_handle_t)0, &esph_touchpad_io_config, &esph_touchpad_io_handle);
		ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst816s(esph_touchpad_io_handle, &esph_touchpad_cfg, &touch_handle));
	}

	ESP_LOGI(TAG, "Turn on LCD backlight");
	gpio_set_level(BSP_LCD_BK_LIGHT_GPIO_NUM, BSP_LCD_BK_LIGHT_ON_LEVEL);

	ESP_LOGI(TAG, "Initialize LVGL library");
	lv_init();
	// alloc draw buffers used by LVGL
	// it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized
	lv_color_t *buf1 = heap_caps_malloc(
		BSP_LVGL_BUFSIZ * sizeof(lv_color_t), 
		// MALLOC_CAP_DMA
		MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
		);
	assert(buf1);
	lv_color_t *buf2 = heap_caps_malloc(
		BSP_LVGL_BUFSIZ * sizeof(lv_color_t), 
		// MALLOC_CAP_DMA
		MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
		);
	assert(buf2);
	// initialize LVGL draw buffers
	lv_disp_draw_buf_init(&disp_buf, buf1, buf2, BSP_LVGL_BUFSIZ);

	ESP_LOGI(TAG, "Register display driver to LVGL");
	lv_disp_drv_init(&disp_drv);
	disp_drv.hor_res = BSP_LCD_HOR_RES;
	disp_drv.ver_res = BSP_LCD_VER_RES;
	disp_drv.flush_cb = bsp_lv_flush_cb;
	disp_drv.drv_update_cb = bsp_lv_port_update_cb;
	disp_drv.draw_buf = &disp_buf;
	disp_drv.user_data = panel_handle;
	lv_disp_t *disp = lv_disp_drv_register(&disp_drv);

	ESP_LOGI(TAG, "Install LVGL tick timer");
	// Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
	const esp_timer_create_args_t lvgl_tick_timer_args = {
		.callback = &bsp_lv_tick_inc_cb,
		.name = "lvgl_tick"};
	esp_timer_handle_t lvgl_tick_timer = NULL;
	ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
	ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, BSP_LVGL_TICK_PERIOD_MS * 1000));

	if (touch_ok == true)
	{
		static lv_indev_drv_t indev_drv; // Input device driver (Touch)
		lv_indev_drv_init(&indev_drv);
		indev_drv.type = LV_INDEV_TYPE_POINTER;
		indev_drv.disp = disp;
		indev_drv.read_cb = bsp_lv_touch_cb;
		indev_drv.user_data = touch_handle;

		lv_indev_drv_register(&indev_drv);
		ESP_LOGI(TAG, "Add touch_handle task");
	}

	osmtx_lvgl = xSemaphoreCreateRecursiveMutex();
	if(osmtx_lvgl == NULL) {
		ESP_LOGE(TAG, "Failed to create LVGL mutex");
		while(1);
	}
}
