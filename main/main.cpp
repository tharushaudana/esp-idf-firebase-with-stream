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

static const char *TAG = "MAIN";

#include "firebase-with-stream.h"

firebase_config_t fb_config;
firebase_credentials_t fb_credentials;

firebase_with_stream firebase;

#include "wifi_utilities.h"

json_stream_parser jparser1;
json_stream_parser jparser2;

firebase_stream stream_01([](char c) {
  if (jparser1.parse(c))
  {
    if (jparser1.path == "/path")
    {
      jparser1.replace_prefix_path("/data", jparser1.value.val);
      return;
    }
    
    printf("%s --> %s\n", jparser1.path.c_str(), jparser1.value.val.c_str());
  }
});

firebase_stream stream_02([](char c) {
  if (jparser2.parse(c))
  {
    if (jparser2.path == "/path")
    {
      jparser2.replace_prefix_path("/data", jparser2.value.val);
      return;
    }
    
    printf("%s --> %s\n", jparser2.path.c_str(), jparser2.value.val.c_str());
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
  /*while (1)
  {
    int64_t t = esp_timer_get_time();
    ESP_LOGI(TAG, "time: %lld", t);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }*/
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

    firebase.begin_stream(&stream_01, "/GHC008109272/Settings/.json");
    firebase.begin_stream(&stream_02, "/GHC008109272/IO/Output/.json");

    xTaskCreate(&_update_test_task, "_update_test_task", 4096, NULL, 5, NULL);

    //xTaskCreate(&_test_task, "_test_task", 4096, NULL, 5, NULL);
}