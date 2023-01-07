#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

void list_dir(char mount_point[])
{
    printf("list_dir\n");

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
  
    // esp_vfs_fat_sdcard_unmount(mount_point, card);
    // ESP_LOGI(TAG, "Card unmounted");
}
