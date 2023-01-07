#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "lvgl.h"
#include "bsp_storage.h"
#include "bsp_board.h"
#include "bsp_btn.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "helpers.h"
#include "setdatetime.h"

#include "lv_examples.h"
#include "ispeak.h"
#include "ui_input_datetime.h"
#include "clock_ui.h"
#include "settingsdotcom.h"
#include "tts_helpers.h"
#include "play_mp3_control_example.h"


bool clock_setting;

static const char *TAG = "ui_input_datetime";
void ui_clock_setting_page(void);
static int hour = 11;
static int min = 15;
static int mon = 11;
static int day = 11;
static int year = 2023;
TM_Data time_data;

static bool am = true;
static lv_obj_t *lbtn_hour;
static lv_obj_t *rbtn_hour;
static lv_obj_t *label_hour;

static lv_obj_t *lbtn_min;
static lv_obj_t *rbtn_min;
static lv_obj_t *label_min;

static lv_obj_t *lbtn_mon;
static lv_obj_t *rbtn_mon;
static lv_obj_t *label_mon;

static lv_obj_t *lbtn_day;
static lv_obj_t *rbtn_day;
static lv_obj_t *label_day;

static lv_obj_t *lbtn_year;
static lv_obj_t *rbtn_year;
static lv_obj_t *label_year;

static lv_obj_t *btn_done;
static lv_obj_t *btn_cancel;

static lv_obj_t *btn_ampm;
static lv_obj_t *label_ampm;

static lv_obj_t *btn_md;
static lv_obj_t *label_md;

static lv_obj_t *slider_vol;
static lv_obj_t * label_vol;

static TaskHandle_t g_lvgl_task_handle;
SemaphoreHandle_t g_guisemaphore;

LV_FONT_DECLARE(lv_font_digits_90b4);

static void lvgl_task(void *pvParam)
{
    (void) pvParam;
    g_guisemaphore = xSemaphoreCreateMutex();

    do {
        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(g_guisemaphore, portMAX_DELAY)) {
            lv_task_handler();
            xSemaphoreGive(g_guisemaphore);
        }
        vTaskDelay(pdMS_TO_TICKS(10));

    } while (true);

    vTaskDelete(NULL);
}

static void event_handler_btn(lv_event_t *event)
{
    lv_obj_t *target = lv_event_get_target(event);
    if (target == lbtn_hour)
        {
            if (time_data.hour > 1)
            {
                time_data.hour--;
            }
            else if (time_data.hour == 1)
            {
                time_data.hour = 12;
                time_data.am = !time_data.am;
                lv_label_set_text(label_ampm, (time_data.am ? "AM" : "PM"));
            } 
            char hrs_str[2];
            tostring(hrs_str, time_data.hour);
            lv_label_set_text(label_hour, hrs_str);

        }
    else if (target == rbtn_hour)
        {
            if (time_data.hour < 12)
            {
                time_data.hour++;
            }
            else if (time_data.hour == 12)
            {
                time_data.hour = 1;
                time_data.am = !time_data.am;
                lv_label_set_text(label_ampm, (time_data.am ? "AM" : "PM"));
            } 
            char hrs_str[2];
            tostring(hrs_str, time_data.hour); 
            lv_label_set_text(label_hour, hrs_str);

        }
        else if (target == lbtn_min)
        {
            time_data.min = (time_data.min > 0) ? (time_data.min - 1) : 59;
            char min_str[2];
            tostring(min_str, time_data.min);
            lv_label_set_text(label_min, min_str);

        }
        else if (target == rbtn_min)
        {
            time_data.min = (time_data.min < 59) ? (time_data.min + 1) : 0;
            char min_str[2];
            tostring(min_str, time_data.min);
            lv_label_set_text(label_min, min_str);

        }
        else if (target == lbtn_mon)
        {
            time_data.mon = (time_data.mon > 1) ? (time_data.mon - 1) : 12;
            char mon_str[2];
            tostring(mon_str, time_data.mon);
            lv_label_set_text(label_mon, mon_str);

        }
        else if (target == rbtn_mon)
        {
            time_data.mon = (time_data.mon < 12) ? (time_data.mon + 1) : 1;
            char mon_str[2];
            tostring(mon_str, time_data.mon);
            lv_label_set_text(label_mon, mon_str);

        }
        else if (target == lbtn_day)
        {
            time_data.day = (time_data.day > 1) ? (time_data.day - 1) : 31;
            char day_str[2];
            tostring(day_str, time_data.day);
            lv_label_set_text(label_day, day_str);

        }
        else if (target == rbtn_day)
        {
            time_data.day = (time_data.day < 31) ? (time_data.day + 1) : 1;
            char day_str[2];
            tostring(day_str, time_data.day);
            lv_label_set_text(label_day, day_str);

        }
        else if (target == lbtn_year)
        {
            time_data.year = (time_data.year > 2023) ? (time_data.year - 1) : 2023;
            char year_str[2];
            tostring(year_str, time_data.year);
            lv_label_set_text(label_year, year_str);

        }
        else if (target == rbtn_year)
        {
            time_data.year = (time_data.year < 2050) ? (time_data.year + 1) : 2050;
            char year_str[2];
            tostring(year_str, time_data.year);
            lv_label_set_text(label_year, year_str);

        }
        else if (target == btn_done)
        {
            if (datevalid(time_data.day, time_data.mon, time_data.year))
            {
                setme_datetime(time_data);
                lv_obj_t *obj = lv_event_get_user_data(event);
                lv_obj_del(obj);
                clock_setting = false;
                activate_board_btns();

            }
            else{
                lv_obj_set_style_text_color(label_day, lv_color_hex(0xff0000), 0);
            }


        }
        else if (target == btn_cancel)
        {
            lv_obj_t *obj = lv_event_get_user_data(event);
            lv_obj_del(obj);
            activate_board_btns();
            clock_setting = false;
        }
        else if (target == btn_ampm)
        {
            time_data.am = !time_data.am;
            lv_label_set_text(label_ampm, (time_data.am ? "AM" : "PM"));


        }
        else if (target == btn_md)
        {
            setting_data.md = !setting_data.md;
            lv_label_set_text(label_md, (setting_data.md ? "Mundane" : "Un-Mundane"));
        }

}

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);

    /*Refresh the text*/
    lv_label_set_text_fmt(label_vol, "%"LV_PRId32, lv_slider_get_value(slider));
    setting_data.music_volume = lv_slider_get_value(slider);
    set_voice_volume(setting_data.music_volume);
}

static void folders_list_cb(lv_event_t *event)
{
    lv_obj_t *folders_list = (lv_obj_t *) event->target;

    uint16_t selected = lv_dropdown_get_selected(folders_list);
    get_mp3_folder(selected);
    get_new_mp3_list();

}

void ui_clock_setting_page(void)
{
    int delta = 45;
    int upgap = 0;
    int i = 0;
    int titlegap = 15;
    time_data = get_clock_time11();

    lv_obj_t *page = lv_obj_create(lv_scr_act());
    lv_obj_set_size(page, lv_obj_get_width(lv_obj_get_parent(page)), lv_obj_get_height(lv_obj_get_parent(page)));
    lv_obj_set_style_border_width(page, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(page, lv_color_make(200, 255, 200), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align(page, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *label_title;
    lv_obj_t *label_lb; 
    lv_obj_t *label_rb;

    // row 0  
    btn_done= lv_btn_create(page);
    label_lb = lv_label_create(btn_done);
    lv_obj_set_size(btn_done, 60, 40);
    lv_obj_align(btn_done, LV_ALIGN_TOP_LEFT, 0, upgap + delta * i++);
    lv_obj_align(label_lb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label_lb, &lv_font_montserrat_14, 0);
    lv_label_set_text(label_lb, "DONE");
    lv_obj_add_event_cb(btn_done, event_handler_btn, LV_EVENT_CLICKED, page);

    btn_cancel= lv_btn_create(page);
    label_lb = lv_label_create(btn_cancel);
    lv_obj_set_size(btn_cancel, 60, 40);
    lv_obj_align(btn_cancel, LV_ALIGN_TOP_RIGHT, 0, lv_obj_get_y_aligned(btn_done));
    lv_obj_align(label_lb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label_lb, &lv_font_montserrat_14, 0);
    lv_label_set_text(label_lb, "CANCEL");
    lv_obj_add_event_cb(btn_cancel, event_handler_btn, LV_EVENT_CLICKED, page);

    btn_ampm= lv_btn_create(page);
    label_ampm = lv_label_create(btn_ampm);
    lv_obj_set_size(btn_ampm, 60, 40);
    lv_obj_align(btn_ampm, LV_ALIGN_TOP_MID, 0, lv_obj_get_y_aligned(btn_done));
    lv_obj_align(label_ampm, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label_ampm, &lv_font_montserrat_14, 0);
    lv_label_set_text(label_ampm, (time_data.am ? "AM" : "PM"));
    lv_obj_add_event_cb(btn_ampm,event_handler_btn, LV_EVENT_CLICKED, NULL);

    // row 1
    lbtn_hour = lv_btn_create(page);
    rbtn_hour = lv_btn_create(page);
    label_hour = lv_label_create(page);
    label_title = lv_label_create(page);
    label_lb = lv_label_create(lbtn_hour);
    label_rb = lv_label_create(rbtn_hour);

    lv_obj_set_size(lbtn_hour, 40, 40);
    lv_obj_set_size(rbtn_hour, 40, 40);
    lv_obj_set_size(label_hour, 80, 40);
    lv_obj_set_size(label_title, 80, 40);

    lv_obj_align(lbtn_hour, LV_ALIGN_TOP_LEFT, 5, upgap + delta * i++);
    lv_obj_align(rbtn_hour, LV_ALIGN_TOP_RIGHT, -5, lv_obj_get_y_aligned(lbtn_hour));
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, -35, lv_obj_get_y_aligned(lbtn_hour) + titlegap);
    lv_obj_align(label_hour, LV_ALIGN_TOP_MID, 45, lv_obj_get_y_aligned(lbtn_hour));

    lv_obj_align(label_lb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(label_rb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label_lb, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_font(label_rb, &lv_font_montserrat_40, 0);
    lv_label_set_text(label_lb, "-");
    lv_label_set_text(label_rb, "+");

    lv_label_set_text(label_title, "Hour:");
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_14, 0);

    char hrs_str[2];
    tostring(hrs_str, time_data.hour);
    ESP_LOGI(TAG, "ui_clock_setting_page hours: %s", hrs_str);
    lv_label_set_text(label_hour, hrs_str);
    lv_obj_set_style_text_font(label_hour, &lv_font_montserrat_40, 0);
    lv_obj_add_event_cb(lbtn_hour, event_handler_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(rbtn_hour, event_handler_btn, LV_EVENT_CLICKED, NULL);

    // row 2
    lbtn_min = lv_btn_create(page);
    rbtn_min = lv_btn_create(page);
    label_min = lv_label_create(page);
    label_title = lv_label_create(page);
    label_lb = lv_label_create(lbtn_min);
    label_rb = lv_label_create(rbtn_min);

    lv_obj_set_size(lbtn_min, 40, 40);
    lv_obj_set_size(rbtn_min, 40, 40);
    lv_obj_set_size(label_min, 80, 40);
    lv_obj_set_size(label_title, 80, 40);

    lv_obj_align(lbtn_min, LV_ALIGN_TOP_LEFT, 5, upgap + delta * i++);
    lv_obj_align(rbtn_min, LV_ALIGN_TOP_RIGHT, -5, lv_obj_get_y_aligned(lbtn_min));
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, -35, lv_obj_get_y_aligned(lbtn_min) + titlegap);
    lv_obj_align(label_min, LV_ALIGN_TOP_MID, 45, lv_obj_get_y_aligned(lbtn_min));

    lv_obj_align(label_lb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(label_rb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label_lb, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_font(label_rb, &lv_font_montserrat_40, 0);
    lv_label_set_text(label_lb, "-");
    lv_label_set_text(label_rb, "+");

    lv_label_set_text(label_title, "Minute:");
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_14, 0);

    char min_str[3];
    if (time_data.min == 0)
    {
        strcpy(min_str,"00");
    }
    else
    {
        tostring(min_str, time_data.min);
    }
    lv_label_set_text(label_min, min_str);
    lv_obj_set_style_text_font(label_min, &lv_font_montserrat_40, 0);
    lv_obj_add_event_cb(lbtn_min, event_handler_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(rbtn_min, event_handler_btn, LV_EVENT_CLICKED, NULL);

    // row 3
    lbtn_mon = lv_btn_create(page);
    rbtn_mon = lv_btn_create(page);
    label_mon = lv_label_create(page);
    label_title = lv_label_create(page);
    label_lb = lv_label_create(lbtn_mon);
    label_rb = lv_label_create(rbtn_mon);

    lv_obj_set_size(lbtn_mon, 40, 40);
    lv_obj_set_size(rbtn_mon, 40, 40);
    lv_obj_set_size(label_mon, 80, 40);
    lv_obj_set_size(label_title, 80, 40);

    lv_obj_align(lbtn_mon, LV_ALIGN_TOP_LEFT, 5, upgap + delta * i++);
    lv_obj_align(rbtn_mon, LV_ALIGN_TOP_RIGHT, -5, lv_obj_get_y_aligned(lbtn_mon));
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, -35, lv_obj_get_y_aligned(lbtn_mon) + titlegap);
    lv_obj_align(label_mon, LV_ALIGN_TOP_MID, 45, lv_obj_get_y_aligned(lbtn_mon));

    lv_obj_align(label_lb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(label_rb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label_lb, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_font(label_rb, &lv_font_montserrat_40, 0);
    lv_label_set_text(label_lb, "-");
    lv_label_set_text(label_rb, "+");

    lv_label_set_text(label_title, "Month:");
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_14, 0);

    char mon_str[2];
    tostring(mon_str, time_data.mon);
    lv_label_set_text(label_mon, mon_str);
    lv_obj_set_style_text_font(label_mon, &lv_font_montserrat_40, 0);
    lv_obj_add_event_cb(lbtn_mon, event_handler_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(rbtn_mon, event_handler_btn, LV_EVENT_CLICKED, NULL);

    // row 4
    lbtn_day = lv_btn_create(page);
    rbtn_day = lv_btn_create(page);
    label_day = lv_label_create(page);
    label_title = lv_label_create(page);
    label_lb = lv_label_create(lbtn_day);
    label_rb = lv_label_create(rbtn_day);

    lv_obj_set_size(lbtn_day, 40, 40);
    lv_obj_set_size(rbtn_day, 40, 40);
    lv_obj_set_size(label_day, 80, 40);
    lv_obj_set_size(label_title, 80, 40);

    lv_obj_align(lbtn_day, LV_ALIGN_TOP_LEFT, 5, upgap + delta * i++);
    lv_obj_align(rbtn_day, LV_ALIGN_TOP_RIGHT, -5, lv_obj_get_y_aligned(lbtn_day));
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, -35, lv_obj_get_y_aligned(lbtn_day) + titlegap);
    lv_obj_align(label_day, LV_ALIGN_TOP_MID, 45, lv_obj_get_y_aligned(lbtn_day));

    lv_obj_align(label_lb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(label_rb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label_lb, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_font(label_rb, &lv_font_montserrat_40, 0);
    lv_label_set_text(label_lb, "-");
    lv_label_set_text(label_rb, "+");

    lv_label_set_text(label_title, "Day:");
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_14, 0);

    char day_str[2];
    tostring(day_str, time_data.day);
    lv_label_set_text(label_day, day_str);
    lv_obj_set_style_text_font(label_day, &lv_font_montserrat_40, 0);
    lv_obj_add_event_cb(lbtn_day, event_handler_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(rbtn_day, event_handler_btn, LV_EVENT_CLICKED, NULL);

    // row 5
    lbtn_year = lv_btn_create(page);
    rbtn_year = lv_btn_create(page);
    label_year = lv_label_create(page);
    label_title = lv_label_create(page);
    label_lb = lv_label_create(lbtn_year);
    label_rb = lv_label_create(rbtn_year);

    lv_obj_set_size(lbtn_year, 40, 40);
    lv_obj_set_size(rbtn_year, 40, 40);
    lv_obj_set_size(label_year, 100, 40);
    lv_obj_set_size(label_title, 80, 40);

    lv_obj_align(lbtn_year, LV_ALIGN_TOP_LEFT, 5, upgap + delta * i++);
    lv_obj_align(rbtn_year, LV_ALIGN_TOP_RIGHT, -5, lv_obj_get_y_aligned(lbtn_year));
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, -35, lv_obj_get_y_aligned(lbtn_year) + titlegap);
    lv_obj_align(label_year, LV_ALIGN_TOP_MID, 25, lv_obj_get_y_aligned(lbtn_year));

    lv_obj_align(label_lb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_align(label_rb, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label_lb, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_font(label_rb, &lv_font_montserrat_40, 0);
    lv_label_set_text(label_lb, "-");
    lv_label_set_text(label_rb, "+");

    lv_label_set_text(label_title, "Year:");
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_14, 0);

    char year_str[4];
    tostring(year_str, time_data.year);
    lv_label_set_text(label_year, year_str);
    lv_obj_set_style_text_font(label_year, &lv_font_montserrat_40, 0);
    lv_obj_add_event_cb(lbtn_year, event_handler_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(rbtn_year, event_handler_btn, LV_EVENT_CLICKED, NULL);

// volume slider
    slider_vol = lv_slider_create(page);
    label_vol = lv_label_create(page);
    lv_obj_set_width(slider_vol, 200); 
    lv_obj_set_size(label_vol, 80, 40);
    lv_obj_align(slider_vol, LV_ALIGN_TOP_MID, 0, upgap + 15 + delta * i++);     
    lv_obj_align(label_vol, LV_ALIGN_TOP_LEFT, 0, lv_obj_get_y_aligned(slider_vol));   
    lv_obj_set_style_text_font(label_vol, &lv_font_montserrat_24, 0);  
    lv_label_set_text_fmt(label_vol, "%"LV_PRId32, setting_data.music_volume);
    lv_obj_add_event_cb(slider_vol, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);     /*Assign an event function*/
    lv_slider_set_value(slider_vol, setting_data.music_volume, LV_ANIM_OFF);
// md button
    btn_md = lv_btn_create(page);
    label_md = lv_label_create(btn_md);
    lv_obj_set_size(btn_md, 200, 40);

    lv_obj_align(btn_md, LV_ALIGN_TOP_MID, 0, upgap + delta * i++);  
    lv_obj_align(label_md, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(label_md, &lv_font_montserrat_24, 0);
    lv_label_set_text(label_md, (setting_data.md ? "Mundane" : "!Mundane"));
    lv_obj_add_event_cb(btn_md,event_handler_btn, LV_EVENT_CLICKED, NULL);

    lv_obj_t *folders_list = lv_dropdown_create(page);
    lv_dropdown_clear_options(folders_list);
    lv_obj_set_width(folders_list, 200);
    lv_obj_align(folders_list, LV_ALIGN_TOP_MID, 0, 10 + upgap + delta * i++);  
    lv_obj_add_event_cb(folders_list, folders_list_cb, LV_EVENT_VALUE_CHANGED, NULL); 

    //populate list
    int ix;
    char folder[30] = "";
    int count = mp3_list_count();
    for (ix = 0; ix < count; ix++)
    {
        get_mp3_folder_4_i(folder, ix);
        lv_dropdown_add_option(folders_list, folder, ix);
    }
    lv_dropdown_set_selected(folders_list, mp3_ix);
}

void user_input(void)
{
    ui_clock_setting_page();

}

void update_clock(void)
{
    time_t now;
    char strftime_buf[64];
    struct tm mytimeinfo;

    time(&now);
    setenv("TZ", "CST-8", 1);
    tzset();

    localtime_r(&now, &mytimeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &mytimeinfo);
    hour = mytimeinfo.tm_hour;
    min = mytimeinfo.tm_min;
    mon = mytimeinfo.tm_mon + 1;
    day = mytimeinfo.tm_mday;
    year = mytimeinfo.tm_year + 1900;
    if (year < 2023)
    {
        hour = 12;
        min = 30;
        mon = 11;
        day = 9;
        year = 2023;
    } 
    char hrs_str[2];
    tostring(hrs_str, hour);
    lv_label_set_text(label_hour, hrs_str);

    char min_str[2];
    tostring(min_str, min);
    lv_label_set_text(label_min, min_str);

    char mon_str[2];
    tostring(mon_str, mon);
    lv_label_set_text(label_mon, mon_str);

    char day_str[2];
    tostring(day_str, day);
    lv_label_set_text(label_day, day_str);

    char year_str[2];
    tostring(year_str, year);
    lv_label_set_text(label_year, year_str);

}

