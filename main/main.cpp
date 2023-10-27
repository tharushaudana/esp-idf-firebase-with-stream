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

json_stream_parser jparser;

firebase_stream stream01("/GHC008109272/Settings/.json", [](char c) {
  if (jparser.parse(c))
  {
    if (jparser.path == "/path")
    {
      jparser.replace_prefix_path("/data", jparser.value.val);
    }
    
    printf("%s --> %s\n", jparser.path.c_str(), jparser.value.val.c_str());
  }
});

static void _update_test_task(void *param)
{
    while (1)
    {
        std::string json_data = 
                        "{"
                        "\"k1\":\"v22\","
                        "\"k2\":\"v23\","
                        "\"k3\":\"v24\""
                        "}";

        firebase.crud.update("/test/o2/.json", json_data.c_str());
        
        size_t s = esp_get_free_heap_size();
        ESP_LOGI(TAG, "free heap: %u", s);

        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

static void _test_task(void *param)
{
    vTaskDelay(20000 / portTICK_PERIOD_MS);
}

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

    firebase.begin_stream(&stream01);
    xTaskCreate(&_update_test_task, "_update_test_task", 4096, NULL, 5, NULL);

    //xTaskCreate(&_test_task, "_test_task", 4096, this, 5, NULL);
}