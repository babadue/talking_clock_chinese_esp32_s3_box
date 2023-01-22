// #include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <math.h>
#include "esp_log.h"
#include "helpers.h"
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>

char dateCN_string[30] = "";
char string[30] = "";
float humidity = 0.;
float temperature = 0.;
Book_Keeper_Rec bkr = 
{
    .inner = {  .time_threshold = 0,
                .time_previous = 0,
                .score = 0,
                .count = 0,
                .count_threshold = 0
            },
    .outer1 = { .time_threshold = 0,
                .time_previous = 0,
                .score = 0,
                .count = 0,
                .count_threshold = 0
            }
};

const char *hrcn = "点";
const char *mincn = "分";
static char *pmcn="下午";
static char *amcn="上午";
static char *twocn= "两";
static char *zerocn= "零";
static char *yearcn= "年";  
static char *weekcn= "周";
static char *daycn= "日";
static char *moncn= "月";
static char *sp= " ";
static char *comma= ",";

void tostring(char str[], int num);
int toint(char []);
static const char *TAG = "helpers";

void print_time_data(TM_Data time_data)
{
    ESP_LOGI(TAG,"-----------print_time_data-----------");
    ESP_LOGI(TAG,"print_time_data time_data.min: %d", time_data.min);
    ESP_LOGI(TAG,"print_time_data time_data.hour: %d", time_data.hour); 
    ESP_LOGI(TAG,"print_time_data time_data.day: %d", time_data.day); 
    ESP_LOGI(TAG,"print_time_data time_data.mon: %d", time_data.mon); 
    ESP_LOGI(TAG,"print_time_data time_data.year: %d", time_data.year); 
    ESP_LOGI(TAG,"print_time_data time_data.wday: %d", time_data.wday);     
    ESP_LOGI(TAG,"print_time_data time_data.am: %d", time_data.am); 
}
 
void tostring(char str[], int num)
{
    int i, rem, len = 0, n;
    n = num;
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    
    char str1[2] = "0";
    switch (len)
    {
    case 0:
        strcpy(str, "0\0");
        break;
    case 1:
        str[len] = '\0';
        strcat(str1, str);
        strcpy(str, str1);
        break;
    case 2:
        str[len] = '\0';
        break;
    default:
        break;
    }

}
 
int toint(char str[])
{
    int len = strlen(str);
    int i, num = 0;
 
    for (i = 0; i < len; i++)
    {
        num = num + ((str[len - (i + 1)] - '0') * pow(10, i));
    }
 
   return num;
}

char tochar(int num)
{
    char str[5];
    int i, rem, len = 0, n;
    n = num;
    while (n != 0)
    {
        len++;
        n /= 10;
    }
    for (i = 0; i < len; i++)
    {
        rem = num % 10;
        num = num / 10;
        str[len - (i + 1)] = rem + '0';
    }
    str[len] = '\0';
    char a = str[0];

    return a;
}

TM_Data get_clock_time11(void)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    setenv("TZ", "CST-8", 1);
    tzset();
    localtime_r(&now, &timeinfo);

    TM_Data tm_data;

    tm_data.hour = (timeinfo.tm_hour > 12) ? (timeinfo.tm_hour-12) : ((timeinfo.tm_hour == 0) ? 12 : timeinfo.tm_hour);
    tm_data.min = timeinfo.tm_min;
    tm_data.day = timeinfo.tm_mday;
    tm_data.mon = timeinfo.tm_mon + 1;
    tm_data.year = ((timeinfo.tm_year + 1900) < 2023) ? 2023 : (timeinfo.tm_year + 1900);  
    tm_data.am = (timeinfo.tm_hour >= 12) ? false : true;
    tm_data.wday = timeinfo.tm_wday;
    
    return tm_data;

}


#define max_yr 9999
#define min_yr 1800

int isleap(int y) 
{
   if((y % 4 == 0) && (y % 100 != 0) && (y % 400 == 0))
      return 1;
   else
      return 0;
}
//Function to check the date is valid or not
int datevalid(int d, int m, int y) 
{
   if(y < min_yr || y > max_yr)
      return 0;
   if(m < 1 || m > 12)
      return 0;
   if(d < 1 || d > 31)
      return 0;
      //Now we will check date according to month
   if( m == 2 ) 
   {
        if(isleap(y)) 
        {
            if(d <= 29)
                return 1;
            else
                return 0;
        }
    }
      //April, June, September and November are with 30 days
    if ( m == 4 || m == 6 || m == 9 || m == 11 )
    {
        if(d <= 30)
            return 1;
         else
            return 0;
    }

    return 1;
}

int get_rand(int upper, int lower)
{

    return (rand() % (upper - lower + 1)) + lower;

}

void rlz(char *o, char *r)
{
    if ((strcmp(o,"0") == 0) || (strcmp(o, "00") == 0))
    {
        strcpy(r,"0");
    }
    else
    {
        int n;
        if ((n = strspn(o, "0")) != 0 && o[n] != '\0')
        {
            strcpy(r, &o[n]);
        }
        else
        {
            strcpy(r,o);
        }
    }

}

const char *cn_weekday(int day)
{
    switch (day)
    {
    case 1:
        return "一";
        break;
    case 2:
        return "二";
        break;
    case 3:
        return "三";
        break;
    case 4:
        return "四";
        break;
    case 5:
        return "五";
        break;
    case 6:
        return "六";
        break;
    case 0:
        return "日";
        break;
    default:
        return "日";
        break;
    }
}


void speak_date(void)
{
    TM_Data time_data = get_clock_time11();

    int yr_tho = time_data.year / 1000;
    int yr_hun = time_data.year % 1000 / 100;
    int yr_ten = time_data.year % 1000 % 100 / 10;
    int yr_dig = time_data.year % 1000 % 100 % 10;

    char speak_str[20] = "";

    char mon_cn[3] = "";
    tostring(mon_cn, time_data.mon);
    strcat(speak_str, mon_cn);
    strcat(speak_str, moncn);
    strcat(speak_str, comma);

    char day_cn[2] = "";
    tostring(day_cn, time_data.day);
    strcat(speak_str, day_cn);
    strcat(speak_str, daycn);

    char wday_cn[1] = "";
    tostring(wday_cn, time_data.wday);
    strcat(speak_str, weekcn);
    strcat(speak_str, (time_data.wday > 0) ? wday_cn : daycn);
    strcpy(string, speak_str);

}

void speak_time()
{
    TM_Data time_data = get_clock_time11();

    char *ampm_cn = (time_data.am) ? (amcn) : pmcn;
        char hour_cn[4] = "";
        
        if (time_data.hour == 2)
        {
            strcat(hour_cn, twocn);
        }
        else if (time_data.hour == 0)
        {
            time_data.hour = 12;
            tostring(hour_cn, time_data.hour);
        }
        else
        {
            tostring(hour_cn, time_data.hour);
        }

        char min_cn[3] = "";
        tostring(min_cn, time_data.min);

        char str_time1[30] = "";
        strcat(str_time1, ampm_cn);
        strcat(str_time1, hour_cn);
        strcat(str_time1, hrcn);
        if ((time_data.min < 10) && (time_data.min > 0))
        {
            strcat(str_time1, zerocn);
        }
        strcat(str_time1, min_cn);
        strcat(str_time1, mincn);
        strcpy(string, str_time1);
}

void get_date(void)
{

    TM_Data time_data = get_clock_time11();

    int yr_tho = time_data.year / 1000;
    int yr_hun = time_data.year % 1000 / 100;
    int yr_ten = time_data.year % 1000 % 100 / 10;
    int yr_dig = time_data.year % 1000 % 100 % 10;

    char speak_str[30] = "";
    char o[1] = "";
    char year_cn[1] = "";
    tostring(o, yr_tho);
    rlz(o,year_cn);
    strcat(speak_str, year_cn);
    year_cn[1] = "";
    tostring(o, yr_hun);
    rlz(o,year_cn);
    strcat(speak_str, year_cn);
    year_cn[1] = "";
    tostring(o, yr_ten);
    rlz(o,year_cn);
    strcat(speak_str, year_cn);
    year_cn[1] = "";
    tostring(o, yr_dig);
    rlz(o,year_cn);
    strcat(speak_str, year_cn);
    strcat(speak_str, sp);
    strcat(speak_str, yearcn);
    strcat(speak_str, sp);

    char mon_cn[3] = "";
    tostring(mon_cn, time_data.mon);
    strcat(speak_str, mon_cn);
    strcat(speak_str, sp);
    strcat(speak_str, moncn);
    strcat(speak_str, sp);

    char day_cn[2] = "";
    tostring(day_cn, time_data.day);
    strcat(speak_str, day_cn);
    strcat(speak_str, sp);
    strcat(speak_str, daycn);
    strcat(speak_str, sp);

    char wday_cn[1] = "";
    tostring(wday_cn, time_data.wday);
    strcat(speak_str, weekcn);
    strcat(speak_str, sp);
    strcat(speak_str, cn_weekday(time_data.wday));
    strcpy(dateCN_string, speak_str);
}