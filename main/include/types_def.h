#pragma once
// #include <stdbool.h>
    typedef struct time_info 
    {
        _Bool am;
        int hour;
        int min;
        int day;
        int mon;
        int year;
        int wday; 
    } TM_Data;

    typedef struct setting_info
    {
        _Bool md;
        int voice_volume;
        int music_volume;
    } Setting_Info;

    // typedef struct test_st
    // {
    //     int d;
    //     int b;
    // }Test_St;

    // typedef struct date_written_cn
    // {
    //     char str[50];
    // } Date_Written_CN;

