#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_check.h"
#include "bsp_board.h"
#include "lvgl/lvgl.h"
#include "bsp_lcd.h"
#include "bsp_btn.h"
#include "ui_input_datetime.h"
#include "DHT22.h"
#include "helpers.h"
#include "play_mp3_control_example.h"
#include "settingsdotcom.h"
#include "loading_page.h"

// int time_to_check_mp3 = 01;
// bool clock_setting = false;
int time_to_read_temp = 01;
// static int temp_sensor_gpio = 38;  // this is the one that is being used

static const char *TAG = "clock_ui";

LV_FONT_DECLARE(number_font_90_4bo);
LV_FONT_DECLARE(sim_font_cn1000an_20);

static TaskHandle_t g_lvgl_task_handle;

void activate_board_btns(void);
static void btn_factory_boot_cb(void *arg);

// /* Creates a semaphore to handle concurrent call to lvgl stuff
//  * If you wish to call *any* lvgl function from other threads/tasks
//  * you should lock on the very same semaphore! */
SemaphoreHandle_t g_guisemaphore;
static lv_obj_t *g_status_bar = NULL;

struct myclock_ui
{
    lv_obj_t *lab_time;
    lv_obj_t *lab_date;
    lv_obj_t *lab_ampm;
    lv_obj_t *lab_temp;
    lv_obj_t *lab_humi;
    lv_obj_t *lab_dateCN;
};
struct myclock_ui elements;

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

void ui_acquire(void)
{
    TaskHandle_t task = xTaskGetCurrentTaskHandle();
    if (g_lvgl_task_handle != task) {
        xSemaphoreTake(g_guisemaphore, portMAX_DELAY);
    }
}

void ui_release(void)
{
    TaskHandle_t task = xTaskGetCurrentTaskHandle();
    if (g_lvgl_task_handle != task) {
        xSemaphoreGive(g_guisemaphore);
    }
}

static void print_time_info(struct tm timeinfo)
{
    ESP_LOGI(TAG,"-----------print_time_info-----------");
    ESP_LOGI(TAG,"print_time_data timeinfo.min: %d", timeinfo.tm_min);
    ESP_LOGI(TAG,"print_time_data timeinfo.hour: %d", timeinfo.tm_hour); 
    // ESP_LOGI(TAG,"print_time_data timeinfo.day: %d", timeinfo.tm_da); 
    // ESP_LOGI(TAG,"print_time_data timeinfo.mon: %d", time_data.mon); 
    // ESP_LOGI(TAG,"print_time_data timeinfo.year: %d", time_data.year); 
    // ESP_LOGI(TAG,"print_time_data timeinfo.wday: %d", time_data.wday);     
    // ESP_LOGI(TAG,"print_time_data timeinfo.am: %d", time_data.am); 
}

static void clock_run_cb(lv_timer_t *timer)
{
    // ESP_LOGI(TAG,"clock_run_cb ");
    TM_Data time_data = get_clock_time11();
    // print_time_data(time_data);
    char s[100];
    int rc;
    struct myclock_ui *elements;
    elements = timer->user_data;

    lv_obj_t *lab_time = elements->lab_time;
    lv_obj_t *lab_date = elements->lab_date;
    lv_obj_t *lab_ampm = elements->lab_ampm;
    lv_obj_t *lab_temp = elements->lab_temp;
    lv_obj_t *lab_humi = elements->lab_humi;
    lv_obj_t *lab_dateCN = elements->lab_dateCN;
    time_t now;
    struct tm timeinfo;
    time(&now);
    setenv("TZ", "CST-8", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    // print_time_info(timeinfo);
    lv_label_set_text_fmt(lab_time, "%02u:%02u", ((timeinfo.tm_hour > 12) ? (timeinfo.tm_hour-12) : ((timeinfo.tm_hour == 0) ? 12 : timeinfo.tm_hour)), time_data.min);
    rc = strftime(s,sizeof(s),"%A, %B %d %Y", &timeinfo);
    lv_label_set_text(lab_date, s);
    lv_label_set_text(lab_ampm, (timeinfo.tm_hour >= 12) ? "PM" : "AM");

    get_date();
    lv_label_set_text(lab_dateCN, dateCN_string);
    time_to_read_temp++;
    if (time_to_read_temp >= 3)
    {
        time_to_read_temp = 0;
        char c[10];
        int ret = readDHT();    
        errorHandler(ret);
        temperature = getTemperature() * 1.8 + 32;
        humidity = getHumidity();
        sprintf(c, "%.1f °", temperature);
        lv_label_set_text(lab_temp, c);
        sprintf(c, "%.1f %s", humidity, "%");
        lv_label_set_text(lab_humi, c);
    }
}

void ui_clock_display(void)
{
    esp_log_level_set("clock_ui", ESP_LOG_NONE);
    ESP_LOGI(TAG,"ui_clock_display ");
    const board_res_desc_t *brd = bsp_board_get_description();
    BaseType_t ret_val = xTaskCreatePinnedToCore(lvgl_task, "lvgl_Task", 6 * 1024, NULL, configMAX_PRIORITIES - 3, &g_lvgl_task_handle, 0);
    ESP_ERROR_CHECK((pdPASS == ret_val) ? ESP_OK : ESP_FAIL);

    setDHTgpio( temp_sensor_gpio);  //set reading pin for temp sensor

    ui_acquire();
    
    elements.lab_time = lv_label_create(lv_scr_act());
    elements.lab_date = lv_label_create(lv_scr_act());
    elements.lab_ampm = lv_label_create(lv_scr_act());
    elements.lab_temp = lv_label_create(lv_scr_act());
    elements.lab_humi = lv_label_create(lv_scr_act());
    elements.lab_dateCN = lv_label_create(lv_scr_act());

    lv_label_set_text(elements.lab_time, "");
    lv_label_set_text(elements.lab_date, "");  
    lv_label_set_text(elements.lab_ampm, "");    
    lv_label_set_text(elements.lab_temp, "");
    lv_label_set_text(elements.lab_humi, "");   
    lv_label_set_text(elements.lab_dateCN, "");      

    //display date at bottom
    lv_obj_align(elements.lab_dateCN, LV_ALIGN_BOTTOM_MID, 0, -35);
    lv_obj_set_style_text_font(elements.lab_dateCN, &sim_font_cn1000an_20, 0); // 28

    lv_obj_align(elements.lab_date, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_text_font(elements.lab_date, &sim_font_cn1000an_20, 0);

    lv_obj_align(elements.lab_time, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_font(elements.lab_time, &number_font_90_4bo, 0);

    lv_obj_align(elements.lab_ampm, LV_ALIGN_RIGHT_MID, -5, 25);
    lv_obj_set_style_text_font(elements.lab_ampm, &lv_font_montserrat_14, 0);

    lv_obj_align(elements.lab_temp, LV_ALIGN_TOP_LEFT, 5, 5);
    lv_obj_set_style_text_font(elements.lab_temp, &lv_font_montserrat_14, 0);

    lv_obj_align(elements.lab_humi, LV_ALIGN_TOP_RIGHT, -5, 5);
    lv_obj_set_style_text_font(elements.lab_humi, &lv_font_montserrat_14, 0);

    lv_timer_t *timer = lv_timer_create(clock_run_cb, 1000, &elements);
    clock_run_cb(timer);

    time_t now;
    struct tm timeinfo;
    time(&now);
    setenv("TZ", "CST-8", 1);
    tzset();
    localtime_r(&now, &timeinfo);
    if ((timeinfo.tm_year + 1900) < 2022)
    {
        clock_setting = true;
        ui_clock_setting_page();
    }
    ESP_LOGI(TAG,"ui_clock_display b4 ui release ");

    starting_loading_page(true);
    ui_release();
    activate_board_btns();
}

static void btn_factory_boot_cb(void *arg)
{
    ui_acquire();
    bsp_lcd_set_backlight(true);
    clock_setting = true;
    ui_clock_setting_page();
    ui_release();

    bsp_btn_register_callback(BOARD_BTN_ID_BOOT, BUTTON_SINGLE_CLICK, NULL, NULL);

}

void activate_board_btns(void)
{
    bsp_btn_register_callback(BOARD_BTN_ID_BOOT, BUTTON_SINGLE_CLICK, btn_factory_boot_cb, "board button");
}