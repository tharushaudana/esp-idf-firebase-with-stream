#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_tls.h"
#include "esp_log.h"
#include "firebase-with-stream.h"

static const char *TAG = "FWS:AUTH";

_firebase_auth::_firebase_auth()
{
}

_firebase_auth::~_firebase_auth()
{
}

void _firebase_auth::_loop_task(void *param) 
{
    while (1)
    {
        ESP_LOGI(TAG, "test");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
}

void _firebase_auth::init() 
{
    //xTaskCreate(&_loop_task, "http_test_task", 4096, NULL, 5, NULL);
}