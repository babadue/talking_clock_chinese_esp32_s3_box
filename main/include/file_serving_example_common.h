/*
 * SPDX-FileCopyrightText: 2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
/* HTTP File Server Example, common declarations

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#include "sdkconfig.h"
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

// esp_err_t example_mount_storage(const char *base_path);

esp_err_t example_start_file_server(const char *base_path);

esp_err_t example_stop_file_server();

// void fs_main(void);

void start_fs(void);

void stop_fs(void);

// void start_wifi(void);
void start_wifi_cb(void (*ptr)());

void stop_wifi(void);

void start_net_ser(void);

// // void example_unmount_storage(const char* base_path);

// // void start_mount_sdcard();

// // void start_unmount_sdcard();



#ifdef __cplusplus
}
#endif
