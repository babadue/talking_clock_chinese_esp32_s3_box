#include "esp_log.h"
#include "bsp_board.h"
#include "lvgl/lvgl.h"
#include "settingsdotcom.h"
#include "helpers.h"

static const char *TAG = "loading_page";

struct loading_page
{
    bool check;
    lv_obj_t *page;
    lv_obj_t * label_count;
    int count;
};

struct loading_page lp_data;


static void loading_page_timer_cb(lv_timer_t *timer)
{
    ESP_LOGI(TAG,"loading_page_timer_cb ");
    struct loading_page *lp_data = timer->user_data;
    char ct_str[2];
    tostring(ct_str, lp_data->count--);
    lv_label_set_text(lp_data->label_count, ct_str);
    if (lp_data->check)
    {
        ESP_LOGI(TAG,"loading_page_timer_cb nocheck");
        if (mp3_list_ready)
        {
            ESP_LOGI(TAG,"loading_page_timer_cb mp3 list is ready");
            // lv_obj_del(page);
            lv_timer_del(timer);
            lv_obj_del(lp_data->page);
        }
    }

}

void starting_loading_page(bool check)
{
    esp_log_level_set(TAG, ESP_LOG_NONE);
    ESP_LOGI(TAG,"starting_loading_page");
    lp_data.page = lv_obj_create(lv_scr_act());
    lv_obj_set_size(lp_data.page, lv_obj_get_width(lv_obj_get_parent(lp_data.page)), lv_obj_get_height(lv_obj_get_parent(lp_data.page)));
    lv_obj_set_style_border_width(lp_data.page, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(lp_data.page, lv_color_make(200, 255, 200), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(lp_data.page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_align(lp_data.page, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t * label_title = lv_label_create(lp_data.page);
    lv_obj_set_size(label_title, 110, 40);
    lv_obj_align(label_title, LV_ALIGN_CENTER, 0, 60);
    lv_obj_set_style_text_font(label_title, &lv_font_montserrat_24, 0);
    // lv_obj_set_style_text_align()
    lv_label_set_text(label_title, "starting");

    lp_data.label_count = lv_label_create(lp_data.page);
    lv_obj_set_size(lp_data.label_count, 40, 40);
    lv_obj_align(lp_data.label_count, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(lp_data.label_count, &lv_font_montserrat_24, 0);
    // lv_obj_set_style_text_align()
    lv_label_set_text(lp_data.label_count, "10");

    // lp_data.page = page;
    lp_data.label_count = lp_data.label_count;
    lp_data.count = 10;
    lp_data.check = check;

    lv_timer_t *timer = lv_timer_create(loading_page_timer_cb, 1000, &lp_data);
    loading_page_timer_cb(timer);
}

