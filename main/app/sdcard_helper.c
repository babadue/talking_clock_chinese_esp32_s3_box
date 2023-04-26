#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "esp_log.h"
#include "settingsdotcom.h"
#include "audio_mem.h"

static const char *TAG = "sdcard_helper";
#define SDCARD_FILE_PREV_NAME "file:/"
#define SDCARD_SCAN_URL_MAX_LENGTH (1024 * 2)

void list_dir(char mount_point[])
{
    printf("list_dir mount_point:%s\n", mount_point);

    // read direct
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
        while ((p_dirent = readdir(p_dir)) != NULL)
        {
            if (p_dirent->d_type == DT_REG)
            {
                f_num++;
                printf("read dir file num: %d dino: %d type: %d name: %s\n", f_num, p_dirent->d_ino, p_dirent->d_type, p_dirent->d_name);
            }
        }
        printf("read dir number of files: %d\n", f_num);
    }
    closedir(p_dir);
}

void list_folders(char mount_point[])
{
    printf("get_folders mount_point:%s\n", mount_point);
    // read direct
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
    closedir(p_dir);
}

int getNumOfMusicFolders()
{
    char *mount_point = "/sdcard/music";
    DIR *p_dir = NULL;
    struct dirent *p_dirent = NULL;
    p_dir = opendir(mount_point);
    uint16_t f_num = 0;
    if (p_dir == NULL)
    {
        printf("opendir 2 error\n");
    }
    else
    {
        while ((p_dirent = readdir(p_dir)) != NULL)
        {
            if (p_dirent->d_type == DT_DIR)
            {
                f_num++;
            }
        }
    }
    ESP_LOGE(TAG, "getNumOfMusicFolders number of mp3 folders: %d", f_num);
    closedir(p_dir);
    return f_num;
}

void get_music_folder_from_list(char folder[20], int ix)
{
    char *mount_point = "/sdcard/music";
    DIR *p_dir = NULL;
    struct dirent *p_dirent = NULL;
    p_dir = opendir(mount_point);
    uint16_t f_num = 0;
    char folders_string[200] = "";
    if (p_dir == NULL)
    {
        printf("opendir 2 error\n");
    }
    else
    {
        while ((p_dirent = readdir(p_dir)) != NULL)
        {
            if (p_dirent->d_type == DT_DIR)
            {
                strcat(folders_string, p_dirent->d_name);
                strcat(folders_string, ",");
                f_num++;
            }
        }
    }
    ESP_LOGE(TAG, "get_music_folder_from_list folders_string: %s", folders_string);
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
        strcat(folder, folders_list[ix]);
    }
    closedir(p_dir);
}

int number_of_files(char mount_point[300])
{
    int count = 0;
    DIR *p_dir = NULL;
    struct dirent *p_dirent = NULL;
    p_dir = opendir(mount_point);
    while ((p_dirent = readdir(p_dir)) != NULL)
    {
        (p_dirent->d_type != DT_DIR) ? count++ : count;
    }
    ESP_LOGE(TAG, "number_of_files count: %d", count);
    closedir(p_dir);
    return count;
}

#define MAX_STR_LEN 25  // Maximum length of each string
#define MAX_NUM_STRS 20 // Maximum number of strings in the array

void get_music_url(char url_string[100])
{
    char album[20] = "";
    get_music_folder_from_list(album, mp3_ix);
    ESP_LOGE(TAG, "get_music_url mp3_ix: %d file_ix: %d album: %s", mp3_ix, file_ix, album);
    char *mount_point = audio_calloc(1, SDCARD_SCAN_URL_MAX_LENGTH);
    memset(mount_point, 0, SDCARD_SCAN_URL_MAX_LENGTH);
    sprintf(mount_point, "%s/%s", "/sdcard/music", album);

    char files_list[MAX_NUM_STRS][MAX_STR_LEN];
    DIR *p_dir = NULL;
    struct dirent *p_dirent = NULL;
    p_dir = opendir(mount_point);
    ESP_LOGE(TAG, "get_music_url 1 url_string: %s", url_string);
    ESP_LOGE(TAG, "get_music_url 2 url_string: %s", url_string);
    sprintf(url_string, "%s%s/", SDCARD_FILE_PREV_NAME, mount_point);
    int i = 0;
    if (p_dir == NULL)
    {
        printf("opendir 2 error\n");
    }
    else
    {
        while ((p_dirent = readdir(p_dir)) != NULL)
        {
            if (p_dirent->d_type != DT_DIR)
            {
                strcpy(files_list[i++], p_dirent->d_name);
            }
        }
    }
    // for (int x = 0; x < i; x++)
    // {
    //     ESP_LOGE(TAG, "11 get_music_url x: %d mp3 file: %s", x, files_list[x]);
    // }
    if ((file_ix < 0) || (file_ix >= i))
    {
        ESP_LOGE(TAG, "get_music_url file_ix: %d > i: %d", file_ix, i);
        ESP_LOGE(TAG, "get_music_url 3a url_string: %s ", url_string);
        strcat(url_string, files_list[file_ix = 0]);
        ESP_LOGE(TAG, "get_music_url 3b url_string: %s ", url_string);
    }
    else if (file_ix < i)
    {
        ESP_LOGE(TAG, "get_music_url file_ix: %d < i: %d", file_ix, i);
        ESP_LOGE(TAG, "get_music_url 4a url_string: %s ", url_string);
        strcat(url_string, files_list[file_ix]);
        ESP_LOGE(TAG, "get_music_url 4b url_string: %s ", url_string);
    }

    audio_free(mount_point);
    closedir(p_dir);
}
