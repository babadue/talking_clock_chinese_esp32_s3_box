#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/i2s.h"
#include "esp_tts.h"
#include "esp_tts_voice_xiaole.h"
#include "esp_tts_voice_template.h"
#include "esp_tts_player.h"
#include "tts_helpers.h"
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "helpers.h"
#include "alt_scores.h"
#include "play_mp3_control_example.h"
#include "tts_helpers.h"
#include "settingsdotcom.h"
#include "spi_flash_mmap.h"
#include "esp_partition.h"

static const char *TAG = "ispeak";
// Book_Keeper_Rec bkr;
const esp_partition_t* part;
static esp_tts_handle_t *tts_handle;

static char *done="完毕";
const char *responses[] = {"我在", "诺", "哟我在", "哟我来了", "干嘛叫我", "是本人", "本人在", "哎哟", "又什么了", "不要大喊吧", 
                        "小人在", "喂，干吗打扰我", "人类,干嘛", "大人 小人在", "老大", "老板", "废话小说", "哟哟哟", "话不多说", 
                        "果然如此", "你我两个世界", "不必这样吧", "不该说的就不说", "就不说不该说的", "我很忙"};

const char *byewords[] = {"我去休息了", "我太累了", "我去忙了", "不要打扰我", "后会有其", "再见了吧", "我不干了", "不做了"};

void initializeBoard()
{
    esp_log_level_set("ispeak", ESP_LOG_NONE);
    create_keeper();
 
    part=esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, "voice_data");
    if (part==NULL) { 
        ESP_LOGI(TAG,"Couldn't find voice data partition!");
        // return 0;
    }
    // else {
    //     printf("voice_data paration size:%d\n", part->size);
    // }

    ESP_ERROR_CHECK(esp_board_init(AUDIO_HAL_16K_SAMPLES, 1, 16));
    spi_flash_mmap_handle_t mmap;
    uint16_t* voicedata;
    esp_err_t err=esp_partition_mmap(part, 0, part->size, SPI_FLASH_MMAP_DATA, &voicedata, &mmap);
    if (err != ESP_OK) {
        ESP_LOGI(TAG,"Couldn't map voice data partition!\n"); 
        // return 0;
    }
    esp_tts_voice_t *voice=esp_tts_voice_set_init(&esp_tts_voice_template, voicedata); 

    tts_handle=esp_tts_create(voice);
}

void speaktext(char *cmd)
{
    set_voice_volume(setting_data.voice_volume);
    ESP_LOGI(TAG,"speaktext command %s voice_volume: %d", cmd, setting_data.voice_volume);
    if (esp_tts_parse_chinese(tts_handle, cmd)) {
            int len[1]={0};
            do {
                short *pcm_data=esp_tts_stream_play(tts_handle, len, 3);
                esp_audio_play(pcm_data, len[0]*2, portMAX_DELAY);
            } while(len[0]>0);
    }
}

// start task 
static void speak_task(void *arg)
{
    char *speak_str = arg;
    while (true) {
        ESP_LOGI(TAG, "speak_task core id is: %d", xPortGetCoreID());
        speaktext(speak_str);
        ESP_LOGI(TAG, "speak_task speak_str: %s", speak_str);
        ESP_LOGI(TAG, "speak_task afyter speaking");
        break;
    }
    ESP_LOGI(TAG, "speak_task reset tts_busy_talking");
    vTaskDelete(NULL);
}

// start task start
static void speak_task_start(void *arg)
{
    // BaseType_t ret_val = xTaskCreatePinnedToCore(speak_task, "speak Task", 4 * 1024, (void*)arg, configMAX_PRIORITIES - 3, NULL, 1);
    BaseType_t ret_val = xTaskCreatePinnedToCore(speak_task, "speak Task", 4 * 1024, (void*)arg, 5, NULL, 1);
    ESP_ERROR_CHECK_WITHOUT_ABORT((pdPASS == ret_val) ? ESP_OK : ESP_FAIL);
}

void speak_time_cn()
{
    switch (mp3_player_state())
    {
        case AEL_STATE_NONE:
            ESP_LOGI(TAG, "speak_time_cn case AEL_STATE_NONE:%d", AEL_STATE_NONE);
            speak_time();
            // speaktext(string);
            speak_task_start(string);
            break;
        case AEL_STATE_INIT:
            ESP_LOGI(TAG, "speak_time_cn case AEL_STATE_INIT:%d", AEL_STATE_INIT);
            speak_time();
            // speaktext(string);
            speak_task_start(string);
            break;
        case AEL_STATE_RUNNING:
            ESP_LOGI(TAG, "speak_time_cn case AEL_STATE_RUNNING:%d", AEL_STATE_RUNNING);
            break;
        case AEL_STATE_PAUSED:
            ESP_LOGI(TAG, "speak_time_cn case AEL_STATE_PAUSED:%d", AEL_STATE_PAUSED);
            speak_time();
            // speaktext(string);
            speak_task_start(string);
            break;
        default:
            ESP_LOGI(TAG, "speak_time_cn case AEL_STATE_-1");
            speak_time();
            // speaktext(string);
            speak_task_start(string);
            break;
    }
}

void speak_date_cn(void)
{
    switch (mp3_player_state())
    {
        case AEL_STATE_NONE:
            ESP_LOGI(TAG, "speak_date_cn case AEL_STATE_NONE:%d", AEL_STATE_NONE);
            speak_date();
            // speaktext(string);
            speak_task_start(string);
            break;
        case AEL_STATE_INIT:
            ESP_LOGI(TAG, "speak_date_cn case AEL_STATE_INIT:%d", AEL_STATE_INIT);
            speak_date();
            // speaktext(string);
            speak_task_start(string);
            break;
        case AEL_STATE_RUNNING:
            ESP_LOGI(TAG, "speak_date_cn AEL_STATE_RUNNING:%d", AEL_STATE_RUNNING);
            break;
        case AEL_STATE_PAUSED:
            ESP_LOGI(TAG, "speak_date_cn case AEL_STATE_PAUSED:%d", AEL_STATE_PAUSED);
            speak_date();
            // speaktext(string);
            speak_task_start(string);
            break;
        default:
            ESP_LOGI(TAG, "speak_date_cn case AEL_STATE_-1");
            speak_date();
            // speaktext(string);
            speak_task_start(string);
            break;
    }
}

void speak_temp()
{
    strcpy(string, "");
    switch (mp3_player_state())
    {
        case AEL_STATE_NONE:
            ESP_LOGI(TAG, "speak_ww case AEL_STATE_NONE:%d", AEL_STATE_NONE);
            sprintf(string, "%.1f 度", temperature);
            ESP_LOGI(TAG, "temp to speak speak_str is: %s", string);
            speak_task_start(string);
            break;
        case AEL_STATE_INIT:
            ESP_LOGI(TAG, "speak_ww case AEL_STATE_INIT:%d", AEL_STATE_INIT);
            sprintf(string, "%.1f 度", temperature);
            ESP_LOGI(TAG, "temp to speak speak_str is: %s", string);
            speak_task_start(string);
            break;
        case AEL_STATE_RUNNING:
            ESP_LOGI(TAG, "speak_ww case AEL_STATE_RUNNING:%d", AEL_STATE_RUNNING);
            break;
        case AEL_STATE_PAUSED:
            ESP_LOGI(TAG, "speak_ww case AEL_STATE_PAUSED:%d", AEL_STATE_PAUSED);
            sprintf(string, "%.1f 度", temperature);
            ESP_LOGI(TAG, "temp to speak speak_str is: %s", string);
            speak_task_start(string);
            break;
        default:
            ESP_LOGI(TAG, "speak_ww case AEL_STATE_-1");
            sprintf(string, "%.1f 度", temperature);
            ESP_LOGI(TAG, "temp to speak speak_str is: %s", string);
            speak_task_start(string);
            break;
    }
}

void speak_ww()
{
    ESP_LOGI(TAG, "speak_ww ");
    switch (mp3_player_state())
    {
        case AEL_STATE_NONE:
            ESP_LOGI(TAG, "speak_ww case AEL_STATE_NONE:%d", AEL_STATE_NONE);
            // speaktext(responses[book_keeper_ww()]);
            speak_task_start(responses[book_keeper_ww()]);
            break;
        case AEL_STATE_INIT:
            ESP_LOGI(TAG, "speak_ww case AEL_STATE_INIT:%d", AEL_STATE_INIT);
            // speaktext(responses[book_keeper_ww()]);
            speak_task_start(responses[book_keeper_ww()]);
            break;
        case AEL_STATE_RUNNING:
            ESP_LOGI(TAG, "speak_ww case AEL_STATE_RUNNING:%d", AEL_STATE_RUNNING);
            pause_mp3();
            ESP_LOGI(TAG, "speak_ww case AEL_STATE_RUNNING:%d after pausing", AEL_STATE_RUNNING);
            speaktext(responses[book_keeper_ww()]);
            // speak_task_start(responses[book_keeper_ww()]);
            break;
        case AEL_STATE_PAUSED:
            ESP_LOGI(TAG, "speak_ww case AEL_STATE_PAUSED:%d", AEL_STATE_PAUSED);
            // speaktext(responses[book_keeper_ww()]);
            speak_task_start(responses[book_keeper_ww()]);
            break;
        default:
            ESP_LOGI(TAG, "speak_ww case AEL_STATE_-1");
            // speaktext(responses[book_keeper_ww()]);
            speak_task_start(responses[book_keeper_ww()]);
            break;
    } 
}

void speak_byeword()
{
    ESP_LOGI(TAG, "speak_byeword");
    switch (mp3_player_state())
    {
    case AEL_STATE_NONE:
        ESP_LOGI(TAG, "speak_byeword case AEL_STATE_NONE:%d", AEL_STATE_NONE);
        speaktext(byewords[book_keeper_bw()]);
        break;
    case AEL_STATE_INIT:
        ESP_LOGI(TAG, "speak_byeword case AEL_STATE_INIT:%d", AEL_STATE_INIT);
        speaktext(byewords[book_keeper_bw()]);  
        break;
    case AEL_STATE_RUNNING:
        ESP_LOGI(TAG, "speak_byeword case AEL_STATE_RUNNING:%d", AEL_STATE_RUNNING);
        break;
    case AEL_STATE_PAUSED:
        ESP_LOGI(TAG, "speak_byeword case AEL_STATE_PAUSED:%d", AEL_STATE_PAUSED);
        speaktext(byewords[book_keeper_bw()]);
        resume_mp3();
        break;
    default:
        ESP_LOGI(TAG, "speak_byeword case AEL_STATE_-1");
        if (!is_mp3_running)
        {
            speaktext(byewords[book_keeper_bw()]);  
            ESP_LOGI(TAG, "speak_byeword case AEL_STATE_-1 after talking");
        }
        break;
    }   
 
}

void play_music()
{
    ESP_LOGI(TAG, "play_music");
    switch (mp3_player_state())
    {
    case AEL_STATE_NONE:
        ESP_LOGI(TAG, "play_music case AEL_STATE_NONE:%d", AEL_STATE_NONE);
        play_mp3(); 
        break;
    case AEL_STATE_INIT:
        ESP_LOGI(TAG, "play_music case AEL_STATE_INIT:%d", AEL_STATE_INIT);
        play_mp3(); 
        break;
    case AEL_STATE_RUNNING:
        ESP_LOGI(TAG, "play_music case AEL_STATE_RUNNING:%d", AEL_STATE_RUNNING);
        break;
    case AEL_STATE_PAUSED:
        ESP_LOGI(TAG, "play_music case AEL_STATE_PAUSED:%d", AEL_STATE_PAUSED);
        break;
    default:
        ESP_LOGI(TAG, "play_music case AEL_STATE_-1");
        play_mp3(); 
        break;
    }  
}

void stop_music()
{
    ESP_LOGI(TAG, "stop_music");
    switch (mp3_player_state())
    {
    case AEL_STATE_NONE:
        ESP_LOGI(TAG, "stop_music case AEL_STATE_NONE:%d", AEL_STATE_NONE);
        break;
    case AEL_STATE_INIT:
        ESP_LOGI(TAG, "stop_music case AEL_STATE_INIT:%d", AEL_STATE_INIT);
        break;
    case AEL_STATE_RUNNING:
        ESP_LOGI(TAG, "stop_music case AEL_STATE_RUNNING:%d", AEL_STATE_RUNNING);
        break;
    case AEL_STATE_PAUSED:
        ESP_LOGI(TAG, "stop_music case AEL_STATE_PAUSED:%d", AEL_STATE_PAUSED);
        // speaktext(done);
        speak_task_start(done);
        stop_mp3(); 
        break;
    default:
        ESP_LOGI(TAG, "stop_music case AEL_STATE_-1");
        break;
    }  
}

void previous_music()
{
    ESP_LOGI(TAG, "previous_music");
    switch (mp3_player_state())
    {
    case AEL_STATE_NONE:
        ESP_LOGI(TAG, "previous_music case AEL_STATE_NONE:%d", AEL_STATE_NONE);
        break;
    case AEL_STATE_INIT:
        ESP_LOGI(TAG, "previous_music case AEL_STATE_INIT:%d", AEL_STATE_INIT);
        break;
    case AEL_STATE_RUNNING:
        ESP_LOGI(TAG, "previous_music case AEL_STATE_RUNNING:%d", AEL_STATE_RUNNING);
        break;
    case AEL_STATE_PAUSED:
        ESP_LOGI(TAG, "previous_music case AEL_STATE_PAUSED:%d", AEL_STATE_PAUSED);
        previous_mp3();         
        break;
    default:
        ESP_LOGI(TAG, "previous_music case AEL_STATE_-1");
        break;
    }  
}

void next_music()
{
    switch (mp3_player_state())
    {
    case AEL_STATE_NONE:
        ESP_LOGI(TAG, "next_music case AEL_STATE_NONE:%d", AEL_STATE_NONE);
        break;
    case AEL_STATE_INIT:
        ESP_LOGI(TAG, "next_music case AEL_STATE_INIT:%d", AEL_STATE_INIT);
        break;
    case AEL_STATE_RUNNING:
        ESP_LOGI(TAG, "next_music case AEL_STATE_RUNNING:%d", AEL_STATE_RUNNING);
        break;
    case AEL_STATE_PAUSED:
        ESP_LOGI(TAG, "next_music case AEL_STATE_PAUSED:%d", AEL_STATE_PAUSED);
        next_mp3();         
        break;
    default:
        ESP_LOGI(TAG, "next_music case AEL_STATE_-1");
        break;
    }  
}

void volume_up(void)
{
    ESP_LOGI(TAG, "volume_up");
    switch (mp3_player_state())
    {
    case AEL_STATE_NONE:
        ESP_LOGI(TAG, "volume_up case AEL_STATE_NONE:%d", AEL_STATE_NONE);
        break;
    case AEL_STATE_INIT:
        ESP_LOGI(TAG, "volume_up case AEL_STATE_INIT:%d", AEL_STATE_INIT);
        break;
    case AEL_STATE_RUNNING:
        ESP_LOGI(TAG, "volume_up case AEL_STATE_RUNNING:%d", AEL_STATE_RUNNING);
        set_voice_volume(setting_data.music_volume = setting_data.music_volume + volume_steps); 
        break;
    case AEL_STATE_PAUSED:
        ESP_LOGI(TAG, "volume_up case AEL_STATE_PAUSED:%d", AEL_STATE_PAUSED);
        set_voice_volume(setting_data.music_volume = setting_data.music_volume + volume_steps); 
        resume_mp3();        
        break;
    default:
        ESP_LOGI(TAG, "volume_up case AEL_STATE_-1");
        break;
    }  
}

void volume_down(void)
{
    ESP_LOGI(TAG, "volume_down");    switch (mp3_player_state())
    {
    case AEL_STATE_NONE:
        ESP_LOGI(TAG, "volume_down case AEL_STATE_NONE:%d", AEL_STATE_NONE);
        break;
    case AEL_STATE_INIT:
        ESP_LOGI(TAG, "volume_down case AEL_STATE_INIT:%d", AEL_STATE_INIT);
        break;
    case AEL_STATE_RUNNING:
        set_voice_volume(setting_data.music_volume = setting_data.music_volume - volume_steps); 
        ESP_LOGI(TAG, "volume_down case AEL_STATE_RUNNING:%d vol after: %d", AEL_STATE_RUNNING, setting_data.music_volume);
        break;
    case AEL_STATE_PAUSED:
        ESP_LOGI(TAG, "volume_down case AEL_STATE_PAUSED:%d", AEL_STATE_PAUSED);
        set_voice_volume(setting_data.music_volume = setting_data.music_volume - volume_steps); 
        resume_mp3();        
        break;
    default:
        ESP_LOGI(TAG, "volume_down case AEL_STATE_-1");
        break;
    } 
}

void speak_cmd(int id)
{
    ESP_LOGI(TAG, "speak_ww id: %d ", id);
    switch (id)
    {
        case 0 ... 1:
            ESP_LOGI(TAG, "speak time command id : %d\n", id);
            speak_time_cn();
            break;
        case 2 ... 3:
            ESP_LOGI(TAG, "speak date command id : %d\n", id);
            speak_date_cn();
            break;
        case 4 ... 5:  
            ESP_LOGI(TAG, "speak temp command id : %d\n", id);
            speak_temp();
            break;
        case 6 ... 7:
            ESP_LOGI(TAG, "play music command id : %d\n", id);
            play_music();
            break;
        case 8 ... 9:
            ESP_LOGI(TAG, "stop music command id : %d\n", id);
            stop_music();
            break;
        case 10 ... 11:
            ESP_LOGI(TAG, "previous music  command id : %d\n", id);
            previous_music();
            break;
        case 12 ... 13:
            ESP_LOGI(TAG, "next music  command id : %d\n", id);
            next_music();
            break;
        case 14 ... 15:
            ESP_LOGI(TAG, "volume up  command id : %d\n", id);
            volume_up();
            break;
        case 16 ... 17:
            ESP_LOGI(TAG, "volume down  command id : %d\n", id);
            volume_down();
            break;
        default:
            ESP_LOGI(TAG, "command id default : %d\n", id);
            // speak(id);
            break;
    }
}


