#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_tls.h"
#include "esp_log.h"
#include "firebase-with-stream.h"
#include "http_request.h"
#include "heapless_json_stream_parser.h"

static const char *TAG = "FWS:AUTH";

json_stream_parser jparser;

_firebase_auth::_firebase_auth()
{
}

_firebase_auth::~_firebase_auth()
{
}

void _firebase_auth::_loop_task(void *param) 
{
    _firebase_auth* instance = static_cast<_firebase_auth*>(param);

    while (1)
    {
        instance->_cycle();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }   
}

void _firebase_auth::_sign_in() 
{
    char *buffer = new char[HTTP_MAX_RECV_BUFFER_SIZE + 1];

    std::string url = "https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=" + config->api_key;

    std::string post_data = 
                    "{"
                    "\"email\":\"" + credentials->email + "\","
                    "\"password\":\"" + credentials->password + "\","
                    "\"returnSecureToken\":true"
                    "}";

    http_response_result_t r = http_make_request(url.c_str(), post_data.c_str(), HTTP_METHOD_POST, buffer);

    if (r.failed) {
        ESP_LOGE(TAG, "[SIGN-IN] >>> Failed to open HTTP connection!");
        delete[] buffer;
        return;
    }

    if (r.status_code != 200)
    {
        ESP_LOGE(TAG, "[SIGN-IN] >>> Response failed with code %d", r.status_code);
        size_t s = esp_get_free_heap_size();
        ESP_LOGI(TAG, "free heap: %u", s);
        delete[] buffer;
        return;
    }

    if (r.read_len > 0)
    {
        /*for (uint16_t i = 0; i < r.read_len; i++)
        {
            if (jparser.parse(buffer[i]))
            {
                printf("%s --> %s \n", jparser.path.c_str(), jparser.value.val.c_str());
            }
        }*/
    }

    size_t s = esp_get_free_heap_size();
    ESP_LOGI(TAG, "free heap: %u", s);

    delete[] buffer;
}

void _firebase_auth::_cycle() 
{
    if (!netstat->is_ready()) 
    {
        ESP_LOGE(TAG, "Network is not ready! Unable to continue.");
        return;
    }

    if (_sign_in_required)
    {
        _sign_in();
    }
}

void _firebase_auth::init() 
{
    xTaskCreate(&_loop_task, "http_test_task", 4096, this, 5, NULL);
}