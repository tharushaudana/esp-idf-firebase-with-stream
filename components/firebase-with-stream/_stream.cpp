#include "firebase-with-stream.h"

static const char *TAG = "FWS:STREAM";

firebase_stream::firebase_stream(on_firebase_stream_data_cb_t cb) : eparser("event", "data")
{
    _cb = cb;
}

firebase_stream::~firebase_stream()
{
    delete[] path;
}

void firebase_stream::_loop_task(void *param) 
{
    firebase_stream* instance = static_cast<firebase_stream*>(param);

    ESP_LOGI(TAG, "[path: %s] >>> STARTED.", instance->path);

    while (instance->is_started)
    {
        instance->_run_stream();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }   

    ESP_LOGI(TAG, "[path: %s] >>> STOPPED.", instance->path);

    vTaskDelete(NULL);
}

void firebase_stream::_opened()
{
    _first_data_received = false;
    _opened_at = esp_timer_get_time();
}

bool firebase_stream::_wait_for_first_data()
{
    if (_first_data_received) return true;
    int64_t now = esp_timer_get_time(); 
    return now - _opened_at < _wait_for_first_data_micros;
}

void firebase_stream::_notify_first_data_received()
{
    if (_first_data_received) return;
    _first_data_received = true;
}

void firebase_stream::_check_timeout()
{
    if (is_timedout) return;
    
    int64_t now = esp_timer_get_time();
    
    is_timedout = now - _last_data_at >= _timeout_micros;

    if (is_timedout)
    {
        ESP_LOGW(TAG, "[path: %s] >>> TIMED OUT! Resuming...", path);
    }
}

void firebase_stream::_extend_timeout()
{
    is_timedout = false;
    _last_data_at = esp_timer_get_time();
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
        ESP_LOGW(TAG, "[path: %s] >>> Network is not ready! Unable to continue.", path);
        return;
    }

    if (!token_data->is_valid)
    {
        ESP_LOGW(TAG, "[path: %s] >>> current token is not valid!", path);
        return;
    }

    _extend_timeout();

    std::string url = config->db_url + path + "?auth=" + token_data->id_token;

    char *buffer = new char[1];

    esp_http_client_config_t config = {
        .url = url.c_str(),
        .buffer_size_tx = HTTP_MAX_SEND_BUFFER_SIZE,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_header(client, "Accept", "text/event-stream");

    esp_err_t err;
    if ((err = esp_http_client_open(client, 0)) != ESP_OK) {
        ESP_LOGE(TAG, "[path: %s] >>> Failed to open HTTP connection!", path);
        delete[] buffer;
        return;
    }

    ESP_LOGI(TAG, "[path: %s] >>> Connection opened.", path);

    _opened();

    int content_length =  esp_http_client_fetch_headers(client);
    int status_code = esp_http_client_get_status_code(client);

    if (status_code != 200)
    {
        ESP_LOGE(TAG, "[path: %s] >>> Response failed with code %d", path, status_code);
        delete[] buffer;
        return;
    }

    while (is_started && !is_timedout && netstat->is_ready() && token_data->is_valid)
    {
        int read_len = 0;

        read_len = esp_http_client_read(client, buffer, 1); // set read len to 1 for very instant read.

        if (read_len > 0) 
        {
            if (eparser.event_name_parsed())
            {
                if (eparser.event == "put" || eparser.event == "patch")
                {
                    _extend_timeout();
                }
                else if (eparser.event == "keep-alive")
                {
                    _extend_timeout();       
                }
                else if (eparser.event == "auth_revoked")
                {
                    //token_data->is_valid = false;
                }
                else if (eparser.event == "cancel")
                {
                    
                }
            }

            if (eparser.parse(buffer[0]))
            {
                if (eparser.event == "put" || eparser.event == "patch")
                {
                    _cb(eparser.data);
                    _notify_first_data_received();
                }
            }
        } 
        else
        {
            _check_timeout();

            // check for receiving error. (re-open required)
            if (!_wait_for_first_data()) {
                ESP_LOGE(TAG, "[path: %s] >>> Opened but has receiving error! Re opening...", path);
                break;
            }
        }
    }

    ESP_LOGW(TAG, "[path: %s] >>> Connection closed.", path);

    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    delete[] buffer;
}