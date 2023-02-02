#include "audio_element.h"
void init_pmp3(void);
void pmp3_sdcard(void);
void pause_mp3(void);
void resume_mp3(void);
// bool isPlayingMP3(void);
void stop_mp3(void);
void previous_mp3(void);
void next_mp3(void);
void play_mp3(void);
void check_mp3_player_event(void);
audio_element_state_t mp3_player_state();
void get_new_mp3_list();
void pause_mp3_task();
void get_mp3_playlist();

