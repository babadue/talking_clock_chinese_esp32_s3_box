/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* HTTP File Server Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "string.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "file_serving_example_common.h"
#include "driver/gpio.h"
#include "bsp_board.h"
#include "bsp_btn.h"
#include "softap_example_main.h"
#include "settingsdotcom.h"

#define EXAMPLE_BUTTON_GPIO     0
bool loop1 = false;
const char* base_path = "/sdcard";

/* This example demonstrates how to create file server
 * using esp_http_server. This file has only startup code.
 * Look in file_server.c for the implementation.
 */

static const char *TAG = "file_serving_example_common";

void start_net_ser(void)
{
    ESP_LOGI(TAG, "Starting start_net_ser");
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
}

// start task 
static void wifi_event_task()
{
    while (true) {
        ESP_ERROR_CHECK(example_connect());
        strcpy(server_ip, sta_ip);
        ESP_LOGW(TAG, "start_wifi_internal while true got station ip: %s", server_ip);
        break;
    }
    vTaskDelete(NULL);
}

// start task start
static void wifi_task_start()
{
    BaseType_t ret_val = xTaskCreatePinnedToCore(wifi_event_task, "wifi event task", 4 * 1024, NULL, 5, NULL, 1);
    ESP_ERROR_CHECK_WITHOUT_ABORT((pdPASS == ret_val) ? ESP_OK : ESP_FAIL);
}

void start_wifi_cb(void (*ptr)())
{
    ESP_LOGI(TAG, "Starting start_wifi_cb box");

    if (softAP)
    {
        start_wifi_softAP();
        strcpy(server_ip, "192.168.4.1");
    }
    else
    {
        // wifi station
        ESP_ERROR_CHECK(example_connect());
        strcpy(server_ip, sta_ip);
        ESP_LOGW(TAG, "start_wifi  after example connect got station ip: %s", server_ip);
    }
    (*ptr)();
}

void start_fs(void)
{
    ESP_LOGI(TAG, "Starting start_fs box: %s", base_path);
    const char* base_path = "/sdcard";
    BaseType_t ret_val = (example_start_file_server(base_path));

}

void stop_fs(void)
{
    ESP_LOGI(TAG, "Stoping stop_fs box");
    ESP_ERROR_CHECK(example_stop_file_server());
}

void stop_wifi()
{
    ESP_LOGI(TAG, "stop_wifi");
    if (softAP)
    {
        stop_wifi_softAP();

    }
    else
    {
        // stop wifi station
        example_disconnect();
    }
    
}


