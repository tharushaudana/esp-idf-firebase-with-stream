#include "firebase-with-stream.h"

static const char *TAG = "FWS:STREAM";

event_source_stream_parser eparser("event", "data");

firebase_stream::firebase_stream(const char* path, on_firebase_stream_data_cb_t cb)
{
    _path = path;
    _cb = cb;
}

firebase_stream::~firebase_stream()
{
    delete[] _path;
}

void firebase_stream::_loop_task(void *param) 
{
    firebase_stream* instance = static_cast<firebase_stream*>(param);

    ESP_LOGI(TAG, "[path: %s] >>> STARTED.", instance->_path);

    while (instance->is_started)
    {
        instance->_run_stream();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }   

    ESP_LOGI(TAG, "[path: %s] >>> STOPPED.", instance->_path);

    vTaskDelete(NULL);
}

void firebase_stream::start()
{
    is_started = true;
    xTaskCreate(&_loop_task, "_fb_stream_loop_task", 4096, this, 5, NULL);
}

void firebase_stream::cancel()
{
    is_started = false;
}

void firebase_stream::_run_stream()
{
    if (!netstat->is_ready()) 
    {
        ESP_LOGW(TAG, "[path: %s] >>> Network is not ready! Unable to continue.", _path);
        return;
    }

    if (!token_data->is_valid)
    {
        ESP_LOGW(TAG, "[path: %s] >>> current token is not valid!", _path);
        return;
    }

    std::string url = config->db_url + _path + "?auth=" + token_data->id_token;

    char *buffer = new char[HTTP_MAX_RECV_BUFFER_SIZE + 1];

    esp_http_client_config_t config = {
        .url = url.c_str(),
        .buffer_size_tx = HTTP_MAX_SEND_BUFFER_SIZE,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "Accept", "text/event-stream");

    esp_err_t err;
    if ((err = esp_http_client_open(client, 0)) != ESP_OK) {
        ESP_LOGE(TAG, "[path: %s] >>> Failed to open HTTP connection!", _path);
        delete[] buffer;
        return;
    }

    ESP_LOGI(TAG, "[path: %s] >>> Connection opened.", _path);

    int content_length =  esp_http_client_fetch_headers(client);
    int status_code = esp_http_client_get_status_code(client);

    if (status_code != 200)
    {
        if (status_code == 401) 
        {
            token_data->is_valid = false;
        }
        delete[] buffer;
        return;
    }

    while (is_started && netstat->is_ready() && token_data->is_valid)
    {
        int read_len = 0;

        read_len = esp_http_client_read(client, buffer, HTTP_MAX_RECV_BUFFER_SIZE);

        if (read_len > 0) {
            size_t s = esp_get_free_heap_size();
            ESP_LOGI(TAG, "free heap: %u", s);

            for (uint16_t i = 0; i < read_len; i++)
            {
                if (eparser.parse(buffer[i]))
                {
                    if (eparser.event == "put" || eparser.event == "patch")
                    {
                        _cb(eparser.data);
                    }
                    else if (eparser.event == "keep-alive")
                    {
                        
                    }
                    else if (eparser.event == "auth_revoked")
                    {
                        token_data->is_valid = false;
                    }
                    else if (eparser.event == "cancel")
                    {
                        
                    }
                }
            }

        }
    }

    ESP_LOGW(TAG, "[path: %s] >>> Connection closed.", _path);

    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    delete[] buffer;
}