#include "firebase-with-stream.h"

static const char *TAG = "FWS:CRUD";

_firebase_crud::_firebase_crud()
{
}

_firebase_crud::~_firebase_crud()
{
}

bool _firebase_crud::update(const char* path, const char *json_data) 
{
    if (!netstat->is_ready()) 
    {
        ESP_LOGW(TAG, "[UPDATE] >>> Network is not ready! Unable to continue.");
        return false;
    }

    if (!token_data->is_valid)
    {
        ESP_LOGW(TAG, "[UPDATE] >>> current token is not valid!");
        return false;
    }

    char *buffer = new char[HTTP_MAX_RECV_BUFFER_SIZE + 1];

    std::string url = config->db_url + path + "?auth=" + token_data->id_token;

    http_response_result_t r = http_request::make(url.c_str(), json_data, "application/json", HTTP_METHOD_PATCH, buffer);

    if (r.failed) {
        ESP_LOGE(TAG, "[UPDATE] >>> Failed to open HTTP connection!");
        delete[] buffer;
        return false;
    }

    if (r.status_code != 200)
    {
        ESP_LOGE(TAG, "[UPDATE] >>> Response failed with code %d", r.status_code);

        if (r.status_code == 401)
        {
            token_data->is_valid = false;
        }

        delete[] buffer;
        return false;
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

    ESP_LOGI(TAG, "[UPDATE] >>> path: %s updated.", path);

    delete[] buffer;

    return true;
}