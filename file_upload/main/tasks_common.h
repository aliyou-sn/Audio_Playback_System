/*
 * tasks_common.h
 *
 *  Created on: 26 Jul 2024
 *      Author: apple
 */

#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_

#include <freertos/FreeRTOS.h>
#include <driver/i2s.h>

// WiFi application task
#define WIFI_APP_TASK_STACK_SIZE			4096
#define WIFI_APP_TASK_PRIORITY				5
#define WIFI_APP_TASK_CORE_ID				0

// HTTP Server task
#define HTTP_SERVER_TASK_STACK_SIZE			8192
#define HTTP_SERVER_TASK_PRIORITY			4
#define HTTP_SERVER_TASK_CORE_ID			0

// HTTP Server Monitor task
#define HTTP_SERVER_MONITOR_STACK_SIZE		4096
#define HTTP_SERVER_MONITOR_PRIORITY		3
#define HTTP_SERVER_MONITOR_CORE_ID			0

#define USE_I2S

// speaker settings - if using I2S
#define I2S_SPEAKER_SERIAL_CLOCK GPIO_NUM_14
#define I2S_SPEAKER_LEFT_RIGHT_CLOCK GPIO_NUM_21
#define I2S_SPEAKER_SERIAL_DATA GPIO_NUM_12
//#define I2S_SPEAKDER_SD_PIN GPIO_NUM_5


// button - GPIO 0 is the built in button on most dev boards
#define GPIO_BUTTON GPIO_NUM_0

// i2s speaker pins definition
extern i2s_pin_config_t i2s_speaker_pins;


#endif /* MAIN_TASKS_COMMON_H_ */
