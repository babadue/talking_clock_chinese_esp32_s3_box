#pragma once
#include <stdbool.h>
#include "types_def.h"

extern int temp_sensor_gpio;
extern Setting_Info setting_data;
extern _Bool is_mp3_running;
extern int mp3_player_event_timer;
extern int volume_steps;
extern char mp3_folder[20];
extern int mp3_ix;
extern void setting_me(void);
extern void get_mp3_folder(int ix);
extern int mp3_list_count();
extern void get_mp3_folder_4_i(char *folder, int ix);
// extern bool clock_setting;

// extern Test_St testSt;


