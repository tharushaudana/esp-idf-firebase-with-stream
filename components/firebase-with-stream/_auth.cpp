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
    _firebase_auth* instance = static_cast<_firebase_auth*>(param);

    while (1)
    {
        instance->_cycle();
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }   
}

void _firebase_auth::_sign_in() 
{
    ESP_LOGI(TAG, "[SIGN-IN] >>> Performing...");

    char *buffer = new char[HTTP_MAX_RECV_BUFFER_SIZE + 1];

    std::string url = "https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=" + config->api_key;

    std::string post_data = 
                    "{"
                    "\"email\":\"" + credentials->email + "\","
                    "\"password\":\"" + credentials->password + "\","
                    "\"returnSecureToken\":true"
                    "}";

    http_response_result_t r = http_request::make(url.c_str(), post_data.c_str(), "application/json", HTTP_METHOD_POST, buffer);

    if (r.failed) {
        ESP_LOGE(TAG, "[SIGN-IN] >>> Failed to open HTTP connection!");
        delete[] buffer;
        return;
    }

    if (r.status_code != 200)
    {
        ESP_LOGE(TAG, "[SIGN-IN] >>> Response failed with code %d", r.status_code);
        delete[] buffer;
        return;
    }

    if (r.read_len > 0)
    {
        for (uint16_t i = 0; i < r.read_len; i++)
        {
            if (jparser.parse(buffer[i]))
            {
                if (jparser.path == "/idToken")
                {
                    token_data->id_token = jparser.value.val;
                }
                else if (jparser.path == "/refreshToken")
                {
                    token_data->refresh_token = jparser.value.val;
                }
                else if (jparser.path == "/expiresIn")
                {
                    jparser.value.get_value(token_data->expires_in);
                }   
            }
        }
    }

    token_data->set_valid(true);
    _sign_in_required = false;

    ESP_LOGI(TAG, "[SIGN-IN] >>> Successfully completed.");

    delete[] buffer;
}

void _firebase_auth::_token_refresh() 
{
    ESP_LOGI(TAG, "[TOKEN-REFRESH] >>> Performing...");

    char *buffer = new char[HTTP_MAX_RECV_BUFFER_SIZE + 1];

    std::string url = "https://securetoken.googleapis.com/v1/token?key=" + config->api_key;

    std::string post_data = "grant_type=refresh_token&refresh_token=" + token_data->refresh_token;

    http_response_result_t r = http_request::make(url.c_str(), post_data.c_str(), "application/x-www-form-urlencoded", HTTP_METHOD_POST, buffer);

    if (r.failed) {
        ESP_LOGE(TAG, "[TOKEN-REFRESH] >>> Failed to open HTTP connection!");
        delete[] buffer;
        return;
    }

    if (r.status_code != 200)
    {
        ESP_LOGE(TAG, "[TOKEN-REFRESH] >>> Response failed with code %d", r.status_code);
        delete[] buffer;
        return;
    }

    if (r.read_len > 0)
    {
        for (uint16_t i = 0; i < r.read_len; i++)
        {
            if (jparser.parse(buffer[i]))
            {
                if (jparser.path == "/id_token")
                {
                    token_data->id_token = jparser.value.val;
                }
                else if (jparser.path == "/refresh_token")
                {
                    token_data->refresh_token = jparser.value.val;
                }
                else if (jparser.path == "/expires_in")
                {
                    jparser.value.get_value(token_data->expires_in);
                }   
            }
        }
    }

    token_data->set_valid(true);

    ESP_LOGI(TAG, "[TOKEN-REFRESH] >>> Successfully completed.");

    delete[] buffer;
}

void _firebase_auth::_cycle() 
{
    if (!netstat->is_ready()) 
    {
        ESP_LOGW(TAG, "Network is not ready! Unable to continue.");
        return;
    }

    if (_sign_in_required)
    {
        _sign_in();
    }
    else if (!token_data->is_valid)
    {
        _token_refresh();
    }

    token_data->check(5);
}

void _firebase_auth::init() 
{
    xTaskCreate(&_loop_task, "_fb_auth_loop_task", 4096, this, 5, NULL);
}