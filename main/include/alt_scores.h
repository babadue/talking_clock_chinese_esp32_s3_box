#pragma once
int book_keeper_ww(void);
int book_keeper_bw(void);
void create_keeper();
typedef struct keeper_1
{
    long time_threshold;  //sec
    long time_previous;  //sec
    int score;
    int count;   // only for outerx
    int count_threshold;  //only for  outerx
}Keeper;
typedef struct book_keeper_rec_1
{
    Keeper inner;
    Keeper outer1;

} Book_Keeper_Rec;



