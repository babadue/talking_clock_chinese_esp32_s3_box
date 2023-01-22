#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include <sys/time.h>
#include "esp_log.h"
#include "setdatetime.h"
#include "helpers.h"

static const char *TAG = "setdatetime";

void setme_datetime(TM_Data time_data)
{

    setenv("TZ", "CST-8", 1);
    tzset();
    struct tm st = {.tm_sec = 30,
            .tm_min = time_data.min,
            .tm_hour = (time_data.am) ? ((time_data.hour  == 12) ? 0 : time_data.hour )  : 
            ((time_data.hour == 12) ? time_data.hour : time_data.hour + 12),
            .tm_mday = time_data.day,
            .tm_mon = time_data.mon - 1, 
            .tm_year = time_data.year - 1900, 
            .tm_isdst = 1};


    
    setenv("TZ", "CST-8", 1);
    tzset();
    time_t t = mktime(&st);
    char *testtime = ctime(&t);
    
    setenv("TZ", "CST-8", 1);
    tzset();
    struct timeval tv;
    tv.tv_sec = t; 
    int r;

    setenv("TZ", "CST-8", 1);
    tzset();

    r = settimeofday(&tv, NULL);
    setenv("TZ", "CST-8", 1);
    tzset();

    // time_data = get_clock_time11();

    time_t now;
    char strftime_buf[64];
    struct tm timeinfo;

    time(&now);

    setenv("TZ", "CST-8", 1);
    tzset();

    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

    // printf("setme 6\n");

}

