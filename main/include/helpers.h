// #pragma once
#include <stdio.h>
#include "types_def.h"
#include <stdbool.h>
#include "alt_scores.h"
// #include "text_text.h"
extern float temperature;
extern float humidity;
extern char string[30];
extern char dateCN_string[30];
extern bool clock_setting;

extern Book_Keeper_Rec bkr;

void tostring(char [], int);
int toint(char []);
void mainhelpers(int num);
// void tostring1(char str[], int num);
char tochar(int num);
// void current_datetime(void);

TM_Data get_clock_time11(void);
int datevalid(int d, int m, int y);

int get_rand(int upper, int lower);
void speak_date(void);
void speak_time();
void get_date(void);
void rlz(char *o, char *r);
void print_time_data(TM_Data time_data);
void get_folder_path(char path[]);
// void ui_acquire(void);
// void ui_release(void);
