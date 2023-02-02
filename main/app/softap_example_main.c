/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_mac.h"

/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/

#define EXAMPLE_ESP_WIFI_SSID      "Box_AP"
#define EXAMPLE_ESP_WIFI_PASS      "password"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       4

static const char *TAG = "softAP_example_main";
esp_netif_t *netif;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

// typedef enum {
//     WIFI_AUTH_OPEN = 0,         /**< authenticate mode : open */
//     WIFI_AUTH_WEP,              /**< authenticate mode : WEP */
//     WIFI_AUTH_WPA_PSK,          /**< authenticate mode : WPA_PSK */
//     WIFI_AUTH_WPA2_PSK,         /**< authenticate mode : WPA2_PSK */
//     WIFI_AUTH_WPA_WPA2_PSK,     /**< authenticate mode : WPA_WPA2_PSK */
//     WIFI_AUTH_WPA2_ENTERPRISE,  /**< authenticate mode : WPA2_ENTERPRISE */
//     WIFI_AUTH_WPA3_PSK,         /**< authenticate mode : WPA3_PSK */
//     WIFI_AUTH_WPA2_WPA3_PSK,    /**< authenticate mode : WPA2_WPA3_PSK */
//     WIFI_AUTH_WAPI_PSK,         /**< authenticate mode : WAPI_PSK */
//     WIFI_AUTH_MAX
// } wifi_auth_mode_t;

void start_wifi_softAP(void)
{
    ESP_LOGI(TAG, "1 start_wifi_softAP");

    netif = esp_netif_create_default_wifi_ap();

    ESP_LOGI(TAG, "2 start_wifi_softAP");

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_LOGI(TAG, "3 start_wifi_softAP");
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
    ESP_LOGI(TAG, "4 start_wifi_softAP");

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));
    ESP_LOGI(TAG, "5 start_wifi_softAP");

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
            // .authmode = WIFI_AUTH_WPA2_WPA3_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }
    ESP_LOGI(TAG, "6 start_wifi_softAP");
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_LOGI(TAG, "7 start_wifi_softAP");
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

void stop_wifi_softAP()
{
    ESP_LOGI(TAG, "1 stop_wifi_softAP");
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_LOGI(TAG, "2 stop_wifi_softAP");

    esp_netif_destroy_default_wifi(netif);
    ESP_LOGI(TAG, "3 stop_wifi_softAP");
}

