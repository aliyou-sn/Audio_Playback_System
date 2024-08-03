#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "w25qxx.h"
#include "nvs_flash.h"
#include "wifi_app.h"

#define TAG "w25qxx"

void app_main()
{
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	

	

   // Start Wifi
    wifi_app_start();
	
   //Erase All files
    //w25qxx_t dev;
	//w25qxx_init(&dev);
	// if(w25qxx_eraseAll(&dev, true))
	// {
	 //	ESP_LOGE(TAG, "Erase successful");	
	 //}
    
 }



