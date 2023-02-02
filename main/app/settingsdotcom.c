#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "esp_log.h"
#include "settingsdotcom.h"
#include "helpers.h"
#include "sdcard_helper.h"

int temp_sensor_gpio = 38;
int    volume_steps = 3;
bool    is_mp3_running = false;
int mp3_ix = 0;
bool wifi = false;
bool net_ser = false;
bool mp3_list_ready = false;
bool softAP = true;
char server_ip[16] = "";
bool clock_setting = false;
Setting_Info setting_data =
    {
        .md = false,
        .voice_volume = 75,
        .music_volume = 70
    };

static const char *TAG = "settingsdotcom";
char *sdcard = "/sdcard/music";

void setting_me(void)
{
    ESP_LOGI(TAG,"-----------setting----------");
    strcpy(server_ip, "");
    softAP = true;
    mp3_list_ready = false;
    net_ser = false;
    wifi = false;
    temp_sensor_gpio = 38;
    volume_steps = 3;
    setting_data.md = false;
    setting_data.voice_volume = 75;
    setting_data.music_volume = 70;
    is_mp3_running = false;
    mp3_ix = 0;
}

