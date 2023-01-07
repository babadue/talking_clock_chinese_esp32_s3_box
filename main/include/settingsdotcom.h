#pragma once

#include "types_def.h"


Setting_Info setting_data;
bool is_mp3_running;
int mp3_player_event_timer;
int volume_steps;
char mp3_folder[20];
int mp3_ix;
void setting_me(void);
void get_mp3_folder(int ix);
int mp3_list_count();
void get_mp3_folder_4_i(char *folder, int ix);
int temp_sensor_gpio;
