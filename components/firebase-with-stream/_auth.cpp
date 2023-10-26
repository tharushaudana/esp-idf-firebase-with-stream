#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_tls.h"
#include "esp_log.h"
#include "firebase-with-stream.h"
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

    bool failed = false;

    std::string url = "https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=" + config->api_key;

    esp_http_client_config_t config = {
        .url = url.c_str(),
        .method = HTTP_METHOD_POST,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    std::string post_data = 
                    "{"
                    "\"email\":\"" + credentials->email + "\","
                    "\"password\":\"" + credentials->password + "\","
                    "\"returnSecureToken\":true"
                    "}";

    ESP_LOGI(TAG, "%s", url.c_str());

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data.c_str(), strlen(post_data.c_str()));

    esp_err_t err;

    if ((err = esp_http_client_open(client, post_data.length())) != ESP_OK) {
        ESP_LOGE(TAG, "[SIGN-IN] >>> Failed to open HTTP connection: %s", esp_err_to_name(err));
        failed = true;
    }

    int status_code = -1;
    int content_length = -1;

    if (!failed)
    {
        esp_http_client_write(client, post_data.c_str(), post_data.length());
        content_length = esp_http_client_fetch_headers(client);
        status_code = esp_http_client_get_status_code(client);
    }

    if (!failed && status_code != 200)
    {
        ESP_LOGE(TAG, "[SIGN-IN] >>> Response failed with code %d", status_code);
        failed = true;
    }

    //### read...

    if (!failed)
    {
        int read_len = esp_http_client_read(client, buffer, HTTP_MAX_RECV_BUFFER_SIZE);

        if (read_len > 0)
        {
            for (uint16_t i = 0; i < read_len; i++)
            {
                if (jparser.parse(buffer[i]))
                {
                    printf("%s --> %s \n", jparser.path.c_str(), jparser.value.val.c_str());
                }
            }
        }        
    }

    esp_http_client_close(client);
    esp_http_client_cleanup(client);
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