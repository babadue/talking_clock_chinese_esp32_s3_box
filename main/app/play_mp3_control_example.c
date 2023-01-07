/* Control with a touch pad playing MP3 files from SD Card

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "nvs_flash.h"

#include "audio_element.h"
#include "audio_pipeline.h"
#include "audio_event_iface.h"
#include "audio_common.h"
#include "fatfs_stream.h"
#include "i2s_stream.h"
#include "mp3_decoder.h"
#include "filter_resample.h"

#include "esp_peripherals.h"
#include "periph_sdcard.h"
#include "periph_touch.h"
#include "periph_adc_button.h"
#include "board.h"

#include "sdcard_list.h"
#include "sdcard_scan.h"
#include "sdcard_helper.h"
#include "lvgl/lvgl.h"
#include "settingsdotcom.h"
#include "tts_helpers.h"

int time_to_check_mp3; 
static void mp3_player_event(void);
static const char *TAG = "SDCARD_MP3_CONTROL_EXAMPLE";
int core = 1;
int core1 = 1;
bool mp3_started = false;

audio_pipeline_handle_t pipeline;
audio_element_handle_t i2s_stream_writer, mp3_decoder, fatfs_stream_reader, rsp_handle;
playlist_operator_handle_t sdcard_list_handle = NULL;
esp_periph_set_handle_t set;
audio_event_iface_handle_t evt;
char *url = NULL;
char *sdcard = "/sdcard";

static TaskHandle_t g_lvgl_task_handle;
SemaphoreHandle_t g_guisemaphore;
void ui_acquire(void);
void ui_release(void);
audio_element_state_t mp3_player_state();
static void check_mp3_player_event();

void sdcard_url_save_cb(void *user_data, char *url)
{
    playlist_operator_handle_t sdcard_handle = (playlist_operator_handle_t)user_data;
    esp_err_t ret = sdcard_list_save(sdcard_handle, url);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Fail to save sdcard url to sdcard playlist");
    }
}

void get_mp3_playlist()
{
    char sdcard_path[40] = "";
    strcpy(sdcard_path, sdcard);
    strcat(sdcard_path, mp3_folder);
    // ESP_LOGI(TAG, "pmp3_sdcard sdcard_path: %s", sdcard_path);
    // list_dir(sdcard_path);

    ESP_LOGI(TAG, "Set up a sdcard playlist and scan sdcard music save to it");
    sdcard_list_create(&sdcard_list_handle);
    sdcard_scan(sdcard_url_save_cb, sdcard_path, 0, (const char *[]) {"mp3"}, 1, sdcard_list_handle);
    // sdcard_list_show(sdcard_list_handle);

}

void pmp3_sdcard(void)
{
    esp_log_level_set("SDCARD_MP3_CONTROL_EXAMPLE", ESP_LOG_NONE);
    ESP_LOGI(TAG, "pmp3_sdcard");
    esp_periph_config_t periph_cfg = DEFAULT_ESP_PERIPH_SET_CONFIG();
    periph_cfg.task_core = core1;
    set = esp_periph_set_init(&periph_cfg);
    // audio_board_sdcard_init(set, SD_MODE_1_LINE);
    audio_board_sdcard_init(set, SD_MODE_4_LINE);

}

void get_new_mp3_list_now()
{
    ESP_LOGI(TAG, "get_new_mp3_list_now");
    sdcard_list_destroy(sdcard_list_handle);
    get_mp3_playlist();
}

static void get_new_mp3_list_task(void *arg)
{
    (void) arg;
    while (true) {
        get_new_mp3_list_now();
        break;
    }

    vTaskDelete(NULL);
}

// start task
void get_new_mp3_list(void)
{
    BaseType_t ret_val = xTaskCreatePinnedToCore(get_new_mp3_list_task, "get mp3 Task", 4 * 1024, NULL, 5, NULL, 0);
    ESP_ERROR_CHECK_WITHOUT_ABORT((pdPASS == ret_val) ? ESP_OK : ESP_FAIL);
}

void init_pmp3(void)
{
    ESP_LOGI(TAG, "init_pmp3");
    ESP_LOGI(TAG, "[ 2 ] Start codec chip");
    audio_board_handle_t board_handle = audio_board_init();
    audio_hal_ctrl_codec(board_handle->audio_hal, AUDIO_HAL_CODEC_MODE_DECODE, AUDIO_HAL_CTRL_START);

    ESP_LOGI(TAG, "[4.0] Create audio pipeline for playback");
    audio_pipeline_cfg_t pipeline_cfg = DEFAULT_AUDIO_PIPELINE_CONFIG();
    pipeline = audio_pipeline_init(&pipeline_cfg);
    mem_assert(pipeline);

    ESP_LOGI(TAG, "[4.1] Create i2s stream to write data to codec chip");
    if (mp3_player_state() != AEL_STATE_INIT)
    {
        ESP_LOGI(TAG, "do Create i2s stream to write data to codec chip");
        i2s_stream_cfg_t i2s_cfg = I2S_STREAM_CFG_DEFAULT();
        i2s_cfg.task_core = core;
        i2s_cfg.i2s_port = I2S_NUM_1;   //use by tts = 1
        i2s_cfg.i2s_config.sample_rate = 44100;
        i2s_cfg.type = AUDIO_STREAM_WRITER;
        i2s_stream_writer = i2s_stream_init(&i2s_cfg);
    }

    ESP_LOGI(TAG, "[4.2] Create mp3 decoder to decode mp3 file");
    mp3_decoder_cfg_t mp3_cfg = DEFAULT_MP3_DECODER_CONFIG();
    mp3_cfg.task_core = core;
    mp3_decoder = mp3_decoder_init(&mp3_cfg);

    /* ZL38063 audio chip on board of ESP32-LyraTD-MSC does not support 44.1 kHz sampling frequency,
       so resample filter has been added to convert audio data to other rates accepted by the chip.
       You can resample the data to 16 kHz or 48 kHz.
    */
    ESP_LOGI(TAG, "[4.3] Create resample filter");
    rsp_filter_cfg_t rsp_cfg = DEFAULT_RESAMPLE_FILTER_CONFIG();
    rsp_cfg.task_core = core;
    rsp_cfg.dest_rate = 32000;  
    rsp_cfg.complexity = 1;  //default 2
    rsp_handle = rsp_filter_init(&rsp_cfg);
    
    ESP_LOGI(TAG, "[4.4] Create fatfs stream to read data from sdcard");
    fatfs_stream_cfg_t fatfs_cfg = FATFS_STREAM_CFG_DEFAULT();
    fatfs_cfg.task_core = core;
    fatfs_cfg.type = AUDIO_STREAM_READER;
    fatfs_stream_reader = fatfs_stream_init(&fatfs_cfg);

    ESP_LOGI(TAG, "[4.5] Register all elements to audio pipeline");
    audio_pipeline_register(pipeline, fatfs_stream_reader, "file");
    audio_pipeline_register(pipeline, mp3_decoder, "mp3");
    audio_pipeline_register(pipeline, rsp_handle, "filter");
    audio_pipeline_register(pipeline, i2s_stream_writer, "i2s");

    ESP_LOGI(TAG, "[4.6] Link it together [sdcard]-->fatfs_stream-->mp3_decoder-->resample-->i2s_stream-->[codec_chip]");
    const char *link_tag[4] = {"file", "mp3", "filter", "i2s"};
    audio_pipeline_link(pipeline, &link_tag[0], 4);

}

void pause_mp3(void)
{
    ESP_LOGW(TAG, " pause_mp3");
    audio_pipeline_pause(pipeline);
    is_mp3_running = false;
}

void resume_mp3(void)
{
    ESP_LOGW(TAG, " resume_mp3");
    set_voice_volume(setting_data.music_volume);  
    is_mp3_running = true;
    audio_pipeline_resume(pipeline);
}

void stop_mp3(void)
{
    ESP_LOGW(TAG, " stop_mp3");
    is_mp3_running = false;
    mp3_started = false;
    if (pipeline)
    {        
        ESP_LOGI(TAG, "[ 7 ] Stop audio_pipeline");
        audio_pipeline_stop(pipeline);
        audio_pipeline_wait_for_stop(pipeline);
        audio_pipeline_terminate(pipeline);

        audio_pipeline_unregister(pipeline, mp3_decoder);
        audio_pipeline_unregister(pipeline, i2s_stream_writer);
        audio_pipeline_unregister(pipeline, rsp_handle);

        /* Terminate the pipeline before removing the listener */
        audio_pipeline_remove_listener(pipeline);

        /* Stop all peripherals before removing the listener */
        // esp_periph_set_stop_all(set);  //12-23-22
        audio_event_iface_remove_listener(esp_periph_set_get_event_iface(set), evt);
        ESP_LOGI(TAG, "[ 7 ] Stop all peripherals before removing the listene");

        /* Make sure audio_pipeline_remove_listener & audio_event_iface_remove_listener are called before destroying event_iface */
        audio_event_iface_destroy(evt);

        /* Release all resources */
        audio_pipeline_deinit(pipeline);
        // audio_element_deinit(i2s_stream_writer);  
        audio_element_deinit(mp3_decoder);
        audio_element_deinit(rsp_handle);
        // esp_periph_set_destroy(set);   

        pipeline = NULL;
        mp3_decoder = NULL;
        rsp_handle = NULL;
        evt = NULL;

    }


}

audio_element_state_t mp3_player_state()
{
    audio_element_state_t el_state = audio_element_get_state(i2s_stream_writer);
    ESP_LOGW(TAG, " mp3_player_state el_state: %d", el_state);
    return el_state;
}

void previous_mp3()
{
    set_voice_volume(setting_data.music_volume);  
    sdcard_list_prev(sdcard_list_handle, 1, &url);
    ESP_LOGW(TAG, "previous song URL: %s", url);
    audio_element_set_uri(fatfs_stream_reader, url);
    audio_pipeline_reset_ringbuffer(pipeline);
    audio_pipeline_reset_elements(pipeline);
    audio_pipeline_change_state(pipeline, AEL_STATE_INIT);
    audio_pipeline_run(pipeline);
    is_mp3_running = true;

}

void next_mp3()
{
    set_voice_volume(setting_data.music_volume);  
    sdcard_list_next(sdcard_list_handle, 1, &url);
    ESP_LOGW(TAG, "next song URL: %s", url);
    audio_element_set_uri(fatfs_stream_reader, url);
    audio_pipeline_reset_ringbuffer(pipeline);
    audio_pipeline_reset_elements(pipeline);
    audio_pipeline_change_state(pipeline, AEL_STATE_INIT);
    audio_pipeline_run(pipeline);
    is_mp3_running = true;
}

void activate_listener_mp3(void)
{
    ESP_LOGW(TAG, "activate_listener_mp3");
    audio_event_iface_cfg_t evt_cfg = AUDIO_EVENT_IFACE_DEFAULT_CFG();
    evt = audio_event_iface_init(&evt_cfg);

    ESP_LOGI(TAG, "[5.1] Listen for all pipeline events b4");
    esp_err_t ret = audio_pipeline_set_listener(pipeline, evt);
}

static void mp3_player_listener_task(void *arg)
{
    (void) arg;
    const int STATS_TICKS = pdMS_TO_TICKS(2 * 1000);
    activate_listener_mp3();
        while (mp3_started) {
            check_mp3_player_event();
        }
  
    vTaskDelete(NULL);
}

static void start_mp3player_listener_task()
{
    BaseType_t ret_val = xTaskCreatePinnedToCore(mp3_player_listener_task, "mp3 player listener Task", 4 * 1024, NULL, 5, NULL, 1);
    ESP_ERROR_CHECK_WITHOUT_ABORT((pdPASS == ret_val) ? ESP_OK : ESP_FAIL);

}

static void play_mp3_task(void *arg)
{
    ESP_LOGW(TAG, "play_mp3_task");
    (void) arg;
    const int STATS_TICKS = pdMS_TO_TICKS(3 * 1000);
    // bool start_player_event = false;
    while (true) {
        if (!pipeline)
        {
            init_pmp3();
        }
        sdcard_list_current(sdcard_list_handle, &url);
        ESP_LOGW(TAG, "current song URL: %s", url);
        audio_element_set_uri(fatfs_stream_reader, url);
        audio_pipeline_reset_ringbuffer(pipeline);
        audio_pipeline_reset_elements(pipeline);
        audio_pipeline_change_state(pipeline, AEL_STATE_INIT);
        set_voice_volume(setting_data.music_volume);  
        audio_pipeline_run(pipeline);
        time_to_check_mp3 = 0;
        start_mp3player_listener_task();
        break;
    }
    vTaskDelete(NULL);
}

void play_mp3()
{
    ESP_LOGW(TAG, "play_mp3");
    is_mp3_running = true;
    mp3_started = true;
    BaseType_t ret_val = xTaskCreatePinnedToCore(play_mp3_task, "play mp3 Task", 4 * 1024, NULL, 5, NULL, 1);
    ESP_ERROR_CHECK_WITHOUT_ABORT((pdPASS == ret_val) ? ESP_OK : ESP_FAIL);
}

static void check_mp3_player_event()
{
    /* Handle event interface messages from pipeline
    to set music info and to advance to the next song
    */
    audio_event_iface_msg_t msg;
    esp_err_t ret = audio_event_iface_listen(evt, &msg, portMAX_DELAY);
    if (ret != ESP_OK) 
    {
        ESP_LOGE(TAG, "[ * ] Event interface error : %d", ret);
    }
    else if (msg.source_type == AUDIO_ELEMENT_TYPE_ELEMENT) 
    {
        // ESP_LOGI(TAG, "while 1 if AUDIO_ELEMENT_TYPE_ELEMENT after if decoder");
        // Advance to the next song when previous finishes
        if (msg.source == (void *) i2s_stream_writer && msg.cmd == AEL_MSG_CMD_REPORT_STATUS) 
        {

            audio_element_state_t el_state = audio_element_get_state(i2s_stream_writer);
            if (el_state == AEL_STATE_FINISHED) 
            {
                set_voice_volume(setting_data.music_volume);
                sdcard_list_next(sdcard_list_handle, 1, &url);
                ESP_LOGW(TAG, "check_mp3_player_event URL: %s", url);
                /* In previous versions, audio_pipeline_terminal() was called here. It will close all the elememnt task and when use
                * the pipeline nextcheck_mp3_player_event time, all the tasks should be restart again. It speed too much time when we switch to another music.
                * So we use another method to acheive this as below.
                */
                audio_element_set_uri(fatfs_stream_reader, url);
                audio_pipeline_reset_ringbuffer(pipeline);
                audio_pipeline_reset_elements(pipeline);
                audio_pipeline_change_state(pipeline, AEL_STATE_INIT);
                audio_pipeline_run(pipeline);
            }
        }
    }  // else if ESP_OK
}

// // start task 
// static void player_pause_event_task(void *arg)
// {
//     (void) arg;
//     // const int STATS_TICKS = pdMS_TO_TICKS(1 * 1000);
//     while (true) {
//         pause_mp3();
//         ESP_LOGW(TAG, "player_pause_event_task after pause_mp3");
//         break;
//     }
//     vTaskDelete(NULL);
// }

// // start task start
// static void player_pause_task_start(void *arg)
// {
//     BaseType_t ret_val = xTaskCreatePinnedToCore(player_pause_event_task, "player pause event task", 4 * 1024, NULL, 5, NULL, 1);
//     ESP_ERROR_CHECK_WITHOUT_ABORT((pdPASS == ret_val) ? ESP_OK : ESP_FAIL);
// }

// void pause_mp3_task()
// {

//     player_pause_event_task(NULL);

// }

