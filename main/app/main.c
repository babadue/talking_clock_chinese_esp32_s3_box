/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   v0.13
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_check.h"
#include "bsp_board.h"
#include "bsp_lcd.h"
#include "lvgl.h"
#include "lv_port.h"
#include "ispeech.h"
#include "ispeak.h"
#include "setdatetime.h"
#include "clock_ui.h"
#include "play_mp3_control_example.h"
#include "tts_helpers.h"
#include "settingsdotcom.h"
#include "sdcard_helper.h"
#include "file_serving_example_common.h"
#include "helpers.h"

static const char *TAG = "main";

void app_main()
{
    esp_log_level_set("*", ESP_LOG_NONE);

    // initial sd card
    pmp3_sdcard();

    // initial some variables
    setting_me();

    ESP_LOGI(TAG, "-----------app_main started1-----------");
    //  ESP_LOGI(TAG, "app_main core id is: %d", xPortGetCoreID());

    // initial lcd
    ESP_ERROR_CHECK(bsp_board_init());
    ESP_ERROR_CHECK(bsp_board_power_ctrl(POWER_MODULE_AUDIO, true));
    ESP_ERROR_CHECK(lv_port_init());
    ui_clock_display();
    bsp_lcd_set_backlight(true);

    //  initial tts and sr
    initializeBoard();
    speech();

    // // get mp3 files
    mp3_list_ready = true;
    num_of_albums = getNumOfMusicFolders();
    // ESP_LOGI(TAG, "-----------app_main started1----------- numofalbums: %d", num_of_albums);
    // get_new_mp3_list();
}
