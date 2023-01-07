#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "esp_log.h"
#include "settingsdotcom.h"
static const char *TAG = "settingsdotcom";
const char *mp3_folders[] = {"folder1", "folder2", "folder3", "folder4", "folder5"};

void setting_me(void)
{
    // ESP_LOGI(TAG,"-----------setting----------");
    temp_sensor_gpio = 38;
    volume_steps = 3;
    setting_data.md = false;
    setting_data.voice_volume = 75;
    setting_data.music_volume = 60;
    is_mp3_running = false;
    mp3_player_event_timer = 10;
    strcpy(mp3_folder, "/");
    strcat(mp3_folder, mp3_folders[mp3_ix =0]);       // set default mp3 folder
    // ESP_LOGI(TAG,"-----------setting---------mp3_ix: %d\n", mp3_ix);
}

void get_mp3_folder(int ix)
{
    mp3_ix = ix;
    strcpy(mp3_folder, "/");
    strcat(mp3_folder, mp3_folders[ix]); 

}

void get_mp3_folder_4_i(char *folder, int ix)
{
    strcpy(folder, mp3_folders[ix]); 

}

int mp3_list_count()
{
    int count = sizeof(mp3_folders)/sizeof(mp3_folders[0]);
    return count;
}
