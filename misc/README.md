## <p align=center> Apply Patches

</BR>

You need to apply the `idf_v5.0_freertos.patch` from [ESP-ADF](https://github.com/espressif/esp-adf/tree/master/idf_patches).  &nbsp; But as of this posting, that version is not yet available.  &nbsp; The `idf_v4.4_freertos.patch` version will do.&nbsp;  It will update two files in the freertos component.  &nbsp; The tasks.c and task.h are included here if you don't want to update it yourself.  &nbsp; The correct folders are:

`tasks.c` &nbsp;  &nbsp;    $ESP-IDF\components\freertos\FreeRTOS-Kernel\

`task.h`  &nbsp;&nbsp;&nbsp;&nbsp;  $ESP-IDF\components\freertos\FreeRTOS-Kernel\include\freertos\





<br/>

