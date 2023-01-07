#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "esp_log.h"
#include "helpers.h"
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "alt_scores.h"
#include "settingsdotcom.h"

static const char *TAG = "book_keeper";
Book_Keeper_Rec bkr; 

void print_current_bkr()
{
    ESP_LOGI(TAG, "print_current_bk inner.time_previous:%ld inner.score:%d outer1.time_previous:%ld outer1.count:%d outer1.score: %d", bkr.inner.time_previous,
    bkr.inner.score, bkr.outer1.time_previous, bkr.outer1.count, bkr.outer1.score);

}

void create_keeper()
{
    esp_log_level_set("book_keeper", ESP_LOG_NONE);
    ESP_LOGI(TAG, "create_keeper");    
    bkr.inner.time_previous = 0;
    bkr.inner.time_threshold = 60;  //1min
    bkr.inner.score = 0;

    bkr.outer1.time_previous = 0;
    bkr.outer1.time_threshold = 180;  //3 min
    bkr.outer1.count = 0;
    bkr.outer1.count_threshold = 3;  //# of occurrences
    bkr.outer1.score = 0;

}

int book_keeper_ww()
{
    ESP_LOGI(TAG, "book_keeper");
    int index = 0;
    if (setting_data.md)
    {
        ESP_LOGI(TAG, "book_keeper mundane world!");
        return index;
    }
    time_t current_t;
    time(&current_t);
    ESP_LOGI(TAG, "book_keeper current_t: %ld", current_t);
    long diff_t = difftime(current_t, bkr.inner.time_previous);
    ESP_LOGI(TAG, "book_keeper current_t: %ld inner.time.previous: %ld diff: %ld", current_t, bkr.inner.time_previous, diff_t);
    bkr.inner.time_previous = current_t;

    if (diff_t <= bkr.inner.time_threshold)
    {
        ESP_LOGI(TAG, "book_keeper diff_t <= bkr.inner.time_threshold");
        bkr.inner.score = 1;
    }
    else
    {
        bkr.inner.score = 0;
    }

    diff_t = difftime(current_t, bkr.outer1.time_previous);
    if (bkr.outer1.count > bkr.outer1.count_threshold)
    {
        bkr.outer1.score = 1;
        bkr.outer1.count = 0;
        bkr.outer1.time_previous = current_t;
    }
    else 
    {
        bkr.outer1.count = bkr.outer1.count + 1;
    }
    if (diff_t >= bkr.outer1.time_threshold) //&& (bkr.outer1.count <= bkr.outer1.count_threshold)
    {
        ESP_LOGI(TAG, "book_keeper diff_t >= bkr.outer1.time_threshol:  reset");
        bkr.outer1.time_previous = current_t;
        bkr.outer1.score = 0;
        bkr.outer1.count = 0;
    }
    ESP_LOGI(TAG, "book_keeper current_t: %ld outer1.time.previous: %ld diff: %ld", current_t, bkr.outer1.time_previous, diff_t);

    print_current_bkr();

    int total_score = bkr.inner.score + bkr.outer1.score;

    switch (total_score)
    {
    case 1:
        index = get_rand(3,1);
        break;
    case 2 ... 4:
        index = get_rand(24,4);
        // bkr.inner.score = 0;
        // bkr.outer1.score = 0;
        break;
    
    default:
        break;
    }

    ESP_LOGI(TAG, "book_keeper total_score is :%d index: %d\n", total_score, index);

    return index;

}

int book_keeper_bw(void)
{
    ESP_LOGI(TAG, "book_keeper_ww");
    int index = 0;
    if (setting_data.md)
    {
        ESP_LOGI(TAG, "book_keeper_ww mundane world!");
        return index;
    }
    int total_score = bkr.inner.score + bkr.outer1.score;
    switch (total_score)
    {
        case 1:
            index = get_rand(3,1);
            break;
        case 2 ... 4:
            index = get_rand(7,4);
            bkr.inner.score = 0;
            bkr.outer1.score = 0;
            break;
        default:
            break;
    }

    ESP_LOGI(TAG, "book_keeper_bw total_score is :%d index: %d\n", total_score, index);

    return index;

}





