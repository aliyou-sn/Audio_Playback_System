/*
 * http_server.c
 *
 *  Created on: Oct 20, 2021
 *      Author: kjagu
 */
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "sys/param.h"

#include "http_server.h"
#include "tasks_common.h"
#include "wifi_app.h"
#include "w25qxx.h"

// Tag used for ESP serial console messages
static const char TAG[] = "http_server";

// file upload status
static int g_file_upload_status = FILE_UPLOAD_PENDING;

// HTTP server task handle
static httpd_handle_t http_server_handle = NULL;

// HTTP server monitor task handle
static TaskHandle_t task_http_server_monitor = NULL;

// Queue handle used to manipulate the main queue of events
static QueueHandle_t http_server_monitor_queue_handle;


//w25qxx Initialisation
w25qxx_t dev;

/**
 * ESP32 timer configuration passed to esp_timer_create.
 */
const esp_timer_create_args_t file_upload_reset_args = {
		.callback = &http_server_file_upload_reset_callback,
		.arg = NULL,
		.dispatch_method = ESP_TIMER_TASK,
		.name = "file_upload_reset"
};
esp_timer_handle_t file_upload_reset;

// Embedded files: JQuery, index.html, app.css, app.js and favicon.ico files
extern const uint8_t jquery_3_3_1_min_js_start[]	asm("_binary_jquery_3_3_1_min_js_start");
extern const uint8_t jquery_3_3_1_min_js_end[]		asm("_binary_jquery_3_3_1_min_js_end");
extern const uint8_t index_html_start[]				asm("_binary_index_html_start");
extern const uint8_t index_html_end[]				asm("_binary_index_html_end");
extern const uint8_t app_css_start[]				asm("_binary_app_css_start");
extern const uint8_t app_css_end[]					asm("_binary_app_css_end");
extern const uint8_t app_js_start[]					asm("_binary_app_js_start");
extern const uint8_t app_js_end[]					asm("_binary_app_js_end");
extern const uint8_t favicon_ico_start[]			asm("_binary_favicon_ico_start");
extern const uint8_t favicon_ico_end[]				asm("_binary_favicon_ico_end");

/**
 * Checks the g_file_upload_status and creates the file_upload_reset timer if g_file_upload_status is true.
 */
static void http_server_file_upload_reset_timer(void)
{
	if (g_file_upload_status == FILE_UPLOAD_SUCCESSFUL)
	{
		ESP_LOGI(TAG, "http_server_file_upload_reset_timer: file upload successful starting  update reset timer");

		// Give the web page a chance to receive an acknowledge back and initialize the timer
		ESP_ERROR_CHECK(esp_timer_create(&file_upload_reset_args, &file_upload_reset));
		ESP_ERROR_CHECK(esp_timer_start_once(file_upload_reset, 60000000));
	}
	else
	{
		ESP_LOGI(TAG, "http_server_file_upload_reset_timer: file upload unsuccessful");
	}
}




/**
 * HTTP server monitor task used to track events of the HTTP server
 * @param pvParameters parameter which can be passed to the task.
 */
static void http_server_monitor(void *parameter)
{
	http_server_queue_message_t msg;

	for (;;)
	{
		if (xQueueReceive(http_server_monitor_queue_handle, &msg, portMAX_DELAY))
		{
			switch (msg.msgID)
			{
				case HTTP_MSG_WIFI_CONNECT_INIT:
					ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_INIT");

					break;

				case HTTP_MSG_WIFI_CONNECT_SUCCESS:
					ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_SUCCESS");

					break;

				case HTTP_MSG_WIFI_CONNECT_FAIL:
					ESP_LOGI(TAG, "HTTP_MSG_WIFI_CONNECT_FAIL");

					break;

				case HTTP_MSG_FILE_UPLOAD_SUCCESSFUL:
					ESP_LOGI(TAG, "HTTP_MSG_FILE_UPLOAD_SUCCESSFUL");
					g_file_upload_status = FILE_UPLOAD_SUCCESSFUL;
					http_server_file_upload_reset_timer();

					break;

				case HTTP_MSG_FILE_UPLOAD_FAILED:
					ESP_LOGI(TAG, "HTTP_MSG_FILE_UPLOAD_FAILED");
					g_file_upload_status = FILE_UPLOAD_FAILED;

					break;

				default:
					break;
			}
		}
	}
}

/**
 * Jquery get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_jquery_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "Jquery requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)jquery_3_3_1_min_js_start, jquery_3_3_1_min_js_end - jquery_3_3_1_min_js_start);

	return ESP_OK;
}

/**
 * Sends the index.html page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_index_html_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "index.html requested");

	httpd_resp_set_type(req, "text/html");
	httpd_resp_send(req, (const char *)index_html_start, index_html_end - index_html_start);

	return ESP_OK;
}

/**
 * app.css get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_app_css_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "app.css requested");

	httpd_resp_set_type(req, "text/css");
	httpd_resp_send(req, (const char *)app_css_start, app_css_end - app_css_start);

	return ESP_OK;
}

/**
 * app.js get handler is requested when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_app_js_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "app.js requested");

	httpd_resp_set_type(req, "application/javascript");
	httpd_resp_send(req, (const char *)app_js_start, app_js_end - app_js_start);

	return ESP_OK;
}

/**
 * Sends the .ico (icon) file when accessing the web page.
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK
 */
static esp_err_t http_server_favicon_ico_handler(httpd_req_t *req)
{
	ESP_LOGI(TAG, "favicon.ico requested");

	httpd_resp_set_type(req, "image/x-icon");
	httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_end - favicon_ico_start);

	return ESP_OK;
}

/**
 * Receives the .mp3 file via the web page and upload it to serial flash 
 * @param req HTTP request for which the uri needs to be handled.
 * @return ESP_OK, otherwise ESP_FAIL if timeout occurs and the upload cannot be started.
 */
esp_err_t http_server_file_upload_handler(httpd_req_t *req)
{
	w25qxx_init(&dev);

	int content_length = req->content_len;
	char audio_buff[4096];
	int content_received = 0;
	int recv_len;
	
	uint32_t start_addr = 0x000000;
	
	
	while(content_received < content_length)
	{
		if ((recv_len = httpd_req_recv(req, audio_buff, MIN(content_length-content_received, sizeof(audio_buff)))) < 0)
		{
		    // Check if timeout occurred
			if (recv_len == HTTPD_SOCK_ERR_TIMEOUT)
			{
				ESP_LOGI(TAG, "http_server_file_upload_handler: Socket Timeout");
			}
			ESP_LOGI(TAG, "http_server_file_upload_handler: Error %d", recv_len);
			return ESP_FAIL;
		}
		printf("http_server_file_upload_handler:  RX: %d of %d\r", content_received, content_length);
	
		if(write_audio_data_to_flash(&dev, start_addr, (uint8_t*)audio_buff, recv_len) != ESP_OK)
		{
			return ESP_FAIL; // Exit if writing to flash fails
		}
		start_addr += recv_len;

		
		content_received += recv_len;
        printf("http_server_update_handler: Received and wrote %d of %d bytes\n", content_received, content_length);

	
	}
	
	ESP_LOGI(TAG, "UPOAD DONE!!!");
	
	return ESP_OK;
}

/**
 * Upload status handler responds with the file upload status after the upload is started
 * and responds with the compile time/date when the page is first requested
 * @param req HTTP request for which the uri needs to be handled
 * @return ESP_OK
 */
esp_err_t http_server_upload_status_handler(httpd_req_t *req)
{
	char fileJSON[100];

	ESP_LOGI(TAG, "fileUploadStatus requested");

	sprintf(fileJSON, "{\"upload_status\":%d,\"compile_time\":\"%s\",\"compile_date\":\"%s\"}", g_file_upload_status, __TIME__, __DATE__);

	httpd_resp_set_type(req, "application/json");
	httpd_resp_send(req, fileJSON, strlen(fileJSON));

	return ESP_OK;
}


/**
 * Sets up the default httpd server configuration.
 * @return http server instance handle if successful, NULL otherwise.
 */
static httpd_handle_t http_server_configure(void)
{
	// Generate the default configuration
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

	// Create HTTP server monitor task
	xTaskCreatePinnedToCore(&http_server_monitor, "http_server_monitor", HTTP_SERVER_MONITOR_STACK_SIZE, NULL, HTTP_SERVER_MONITOR_PRIORITY, &task_http_server_monitor, HTTP_SERVER_MONITOR_CORE_ID);

	// Create the message queue
	http_server_monitor_queue_handle = xQueueCreate(3, sizeof(http_server_queue_message_t));

	// The core that the HTTP server will run on
	config.core_id = HTTP_SERVER_TASK_CORE_ID;

	// Adjust the default priority to 1 less than the wifi application task
	config.task_priority = HTTP_SERVER_TASK_PRIORITY;

	// Bump up the stack size (default is 4096)
	config.stack_size = HTTP_SERVER_TASK_STACK_SIZE;

	// Increase uri handlers
	config.max_uri_handlers = 20;

	// Increase the timeout limits
	config.recv_wait_timeout = 10;
	config.send_wait_timeout = 10;

	ESP_LOGI(TAG,
			"http_server_configure: Starting server on port: '%d' with task priority: '%d'",
			config.server_port,
			config.task_priority);

	// Start the httpd server
	if (httpd_start(&http_server_handle, &config) == ESP_OK)
	{
		ESP_LOGI(TAG, "http_server_configure: Registering URI handlers");

		// register query handler
		httpd_uri_t jquery_js = {
				.uri = "/jquery-3.3.1.min.js",
				.method = HTTP_GET,
				.handler = http_server_jquery_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &jquery_js);

		// register index.html handler
		httpd_uri_t index_html = {
				.uri = "/",
				.method = HTTP_GET,
				.handler = http_server_index_html_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &index_html);

		// register app.css handler
		httpd_uri_t app_css = {
				.uri = "/app.css",
				.method = HTTP_GET,
				.handler = http_server_app_css_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &app_css);

		// register app.js handler
		httpd_uri_t app_js = {
				.uri = "/app.js",
				.method = HTTP_GET,
				.handler = http_server_app_js_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &app_js);

		// register favicon.ico handler
		httpd_uri_t favicon_ico = {
				.uri = "/favicon.ico",
				.method = HTTP_GET,
				.handler = http_server_favicon_ico_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &favicon_ico);

		// register fileUpload handler
		httpd_uri_t file_upload = {
				.uri = "/fileUpload",
				.method = HTTP_POST,
				.handler = http_server_file_upload_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &file_upload);

		// register fileUploadStatus handler
		httpd_uri_t file_upload_status = {
				.uri = "/fileUploadStatus",
				.method = HTTP_POST,
				.handler = http_server_upload_status_handler,
				.user_ctx = NULL
		};
		httpd_register_uri_handler(http_server_handle, &file_upload_status);

		return http_server_handle;
	}

	return NULL;
}

void http_server_start(void)
{
	if (http_server_handle == NULL)
	{
		http_server_handle = http_server_configure();
	}
}

void http_server_stop(void)
{
	if (http_server_handle)
	{
		httpd_stop(http_server_handle);
		ESP_LOGI(TAG, "http_server_stop: stopping HTTP server");
		http_server_handle = NULL;
	}
	if (task_http_server_monitor)
	{
		vTaskDelete(task_http_server_monitor);
		ESP_LOGI(TAG, "http_server_stop: stopping HTTP server monitor");
		task_http_server_monitor = NULL;
	}
}

BaseType_t http_server_monitor_send_message(http_server_message_e msgID)
{
	http_server_queue_message_t msg;
	msg.msgID = msgID;
	return xQueueSend(http_server_monitor_queue_handle, &msg, portMAX_DELAY);
}

void http_server_file_upload_reset_callback(void *arg)
{
	ESP_LOGI(TAG, "http_server_file_upload_reset_callback: Timer timed-out, restarting the device");
	esp_restart();
}



















