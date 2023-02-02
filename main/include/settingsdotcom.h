#pragma once
#include <stdbool.h>
#include "types_def.h"

extern bool softAP;
extern bool mp3_list_ready;
extern bool net_ser;
extern bool wifi;
extern int temp_sensor_gpio;
extern Setting_Info setting_data;
extern bool is_mp3_running;
extern int volume_steps;
extern int mp3_ix;
extern void setting_me(void);
extern void get_mp3_folder(int ix);
extern char server_ip[16];
extern char *sdcard;


