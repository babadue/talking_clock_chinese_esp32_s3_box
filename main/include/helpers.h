#include <stdio.h>
#include "types_def.h"
// #include "text_text.h"
float temperature;
float humidity;
char string[30];
char dateCN_string[30];

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
