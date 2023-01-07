#pragma once
    typedef struct time_info 
    {
        bool am;
        int hour;
        int min;
        int day;
        int mon;
        int year;
        int wday; 
    } TM_Data;

    typedef struct setting_info
    {
        bool md;
        int voice_volume;
        int music_volume;
    } Setting_Info;

    // typedef struct date_written_cn
    // {
    //     char str[50];
    // } Date_Written_CN;

