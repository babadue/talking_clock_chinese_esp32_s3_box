#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "esp_log.h"
#include "settingsdotcom.h"


static const char *TAG = "sdcard_helper";

void list_dir(char mount_point[])
{
    printf("list_dir mount_point:%s\n", mount_point);

      //read direct
    DIR *p_dir = NULL;
    struct dirent *p_dirent = NULL;

    p_dir = opendir(mount_point);
    if (p_dir == NULL) 
    {
        printf("opendir error\n");
        // return;
    }
    else 
    {
        printf("opendir successfully\n");

    uint16_t f_num = 0;
    while ((p_dirent = readdir(p_dir)) != NULL) {
        if (p_dirent->d_type == DT_REG) {
            f_num++;
            printf("read dir file num: %d dino: %d type: %d name: %s\n", f_num, p_dirent->d_ino, p_dirent->d_type, p_dirent->d_name);
        }

    }
        printf("read dir number of files: %d\n", f_num);

    }
}

void list_folders(char mount_point[])
{
    printf("get_folders mount_point:%s\n", mount_point);
      //read direct
    DIR *p_dir = NULL;
    struct dirent *p_dirent = NULL;
    p_dir = opendir(mount_point);
    if (p_dir == NULL) 
    {
        printf("opendir 2 error\n");
        // return;
    }
    else 
    {
        printf("opendir 2 successfully\n");
        uint16_t f_num = 0;
        while ((p_dirent = readdir(p_dir)) != NULL) 
        {
            if (p_dirent->d_type == DT_DIR) 
            {
                f_num++;
                printf("read 2 dir folder num: %d dino: %d type: %d name: %s\n", f_num, p_dirent->d_ino, p_dirent->d_type, p_dirent->d_name);
            }

        }
    }
}

void get_music_folder_from_list(char folder[20], int ix)
{
    char *mount_point = "/sdcard/music";
    // printf("get_music_folder_from_list:%s\n", mount_point);

      //read direct
    DIR *p_dir = NULL;
    struct dirent *p_dirent = NULL;
    p_dir = opendir(mount_point);
    uint16_t f_num = 0;    
    char folders_string[100] = "";
    if (p_dir == NULL) 
    {
        printf("opendir 2 error\n");
    }
    else 
    {
        // printf("opendir 2 successfully\n");
        while ((p_dirent = readdir(p_dir)) != NULL) 
        {
            if (p_dirent->d_type == DT_DIR) 
            {
                strcat(folders_string, p_dirent->d_name);
                strcat(folders_string, ",");
                // printf("read 2 dir folder num: %d dino: %d type: %d name: %s \n", f_num, p_dirent->d_ino, p_dirent->d_type, p_dirent->d_name);
                f_num++;

            }

        }
    }
    // printf("get_music_folder_from_list folders_string: %s\n", folders_string); 
    char *folders_list[20] = {};
    int i = 0;
    static char *save;
    char *p = strtok_r(folders_string, ",", &save);
    while (p != NULL)
    {
        folders_list[i++] = p;
        p = strtok_r(NULL, ",", &save);
    }
    // for (i=0; i < f_num; i++)
    // {
    //     ESP_LOGI(TAG,"get_music_folder_from_list i: %d: folder:  %s",i, folders_list[i]);
    // }
    strcpy(folder, "");
    if (ix < f_num)
    {
        // ESP_LOGI(TAG,"get_music_folder_from_list 1 ix: %d: folder:  %s", ix, folder);
        strcat(folder, folders_list[ix]);
    }
    
    // ESP_LOGI(TAG,"get_music_folder_from_list 2 ix: %d: folder:  %s", ix, folder);

}