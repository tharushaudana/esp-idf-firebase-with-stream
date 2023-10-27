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

    token_data->is_valid = true;
    _sign_in_required = false;

    ESP_LOGI(TAG, "[SIGN-IN] >>> Successfully completed.");

    delete[] buffer;

    /*char *buffer = new char[HTTP_MAX_RECV_BUFFER_SIZE + 1];

    std::string url = "https://greenhouse-project-bec1e-default-rtdb.firebaseio.com/test/o1/.json?auth=eyJhbGciOiJSUzI1NiIsImtpZCI6IjBkMGU4NmJkNjQ3NDBjYWQyNDc1NjI4ZGEyZWM0OTZkZjUyYWRiNWQiLCJ0eXAiOiJKV1QifQ.eyJpc3MiOiJodHRwczovL3NlY3VyZXRva2VuLmdvb2dsZS5jb20vZ3JlZW5ob3VzZS1wcm9qZWN0LWJlYzFlIiwiYXVkIjoiZ3JlZW5ob3VzZS1wcm9qZWN0LWJlYzFlIiwiYXV0aF90aW1lIjoxNjk4MzY4MTg3LCJ1c2VyX2lkIjoiWktQTllUZHJZMGd3UXZITDZ4QzZDVzdGM0h4MiIsInN1YiI6IlpLUE5ZVGRyWTBnd1F2SEw2eEM2Q1c3RjNIeDIiLCJpYXQiOjE2OTgzNjgxODcsImV4cCI6MTY5ODM3MTc4NywiZW1haWwiOiJ0aGFydXNoYS51ZGFuYTUyOUBnbWFpbC5jb20iLCJlbWFpbF92ZXJpZmllZCI6ZmFsc2UsImZpcmViYXNlIjp7ImlkZW50aXRpZXMiOnsiZW1haWwiOlsidGhhcnVzaGEudWRhbmE1MjlAZ21haWwuY29tIl19LCJzaWduX2luX3Byb3ZpZGVyIjoicGFzc3dvcmQifX0.hrsey58xwS20WAs8OXhWqs8deuKp2R-yU9O7e6WUsMdPK_B5xGQOUUx8a_DoXepbZc99uAEqPXnjGCzUDs2ER0rrO-SnLjHnmQnwRip-7jwkYea6KcWsJWumwxVlKkpiseZFEOCaasoxTJSgtK2NpjKndeJNWsKL_dquVES-DHMWMx2FvdQmY8Upchr-yZ7lqDeouqOpDE2-zzIqdZRhnEHSwqxDuhRM_vdZQMXN-loPI8dZHagRhq1ZuRyEQP_mJbpoHG0AKeE5o1C9S4GcK22sqKOjh_jOYapKRf2Hstx8qZbKAiIXmRA0xYioTVsLOFxeQZPmmZ7ZNKQ98a9jWw";

    std::string post_data = 
                    "{"
                    "\"k1\":\"v22\","
                    "\"k2\":\"v23\","
                    "\"k3\":\"v24\""
                    "}";

    http_response_result_t r = http_make_request(url.c_str(), post_data.c_str(), HTTP_METHOD_PATCH, buffer);

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
        for (uint16_t i = 0; i < r.read_len; i++)
        {
            if (jparser.parse(buffer[i]))
            {
                printf("%s --> %s \n", jparser.path.c_str(), jparser.value.val.c_str());
            }
        }
    }

    size_t s = esp_get_free_heap_size();
    ESP_LOGI(TAG, "free heap: %u", s);

    delete[] buffer;*/
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

    token_data->is_valid = true;

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
}

void _firebase_auth::init() 
{
    xTaskCreate(&_loop_task, "_fb_auth_loop_task", 4096, this, 5, NULL);
}