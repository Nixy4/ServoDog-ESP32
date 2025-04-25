#include "esp_err.h"
#include "esp_log.h"
#include "esp_camera.h"

static const char* TAG = "BSP-CAM";

int bsp_cam_init(const camera_config_t* ccfg_cam)
{
	if(ccfg_cam==NULL) {
		ESP_LOGE(TAG, "Camera Config is NULL");
		return -1;
	}
	esp_err_t err = esp_camera_init(ccfg_cam);
	if(err != ESP_OK) {
		ESP_LOGE(TAG, "Camera Init Failed : 0x%x", err);
    return -1;
	} else {
		sensor_t *s = esp_camera_sensor_get();
		s->set_vflip(s, 1);
		s->set_hmirror(s, 0);
		ESP_LOGI(TAG, "Camera Init Success");
    return 0;
	}
}