#include <string.h>
#include <sys/param.h>
#include <stdlib.h>
#include <ctype.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

static const char *TAG = "TAG";

#include "firebase-with-stream.h"

firebase_config_t fb_config;
firebase_credentials_t fb_credentials;

firebase_with_stream firebase;

#include "wifi_utilities.h"

extern "C" void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");

    wifi_init_sta();

    fb_config.api_key = "AIzaSyC2KRrFHQpntl4lMpuh-Wql0TB4njcBIlU";
    fb_config.db_url = "https://greenhouse-project-bec1e-default-rtdb.firebaseio.com";

    fb_credentials.email = "tharusha.udana529@gmail.com";
    fb_credentials.password = "tha2003";

    firebase.set_config(fb_config);
    firebase.set_credentials(fb_credentials);

    firebase.begin();
}