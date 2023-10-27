const uint16_t HTTP_MAX_RECV_BUFFER_SIZE = 2048;

struct http_response_result_t
{
    bool failed = false;
    int status_code = -1;
    int content_length = -1;
    int read_len = -1;
};

class http_request
{
private:
    /* data */
public:
    static http_response_result_t make(const char* url, const char* data, const char* content_type, esp_http_client_method_t method, char *&buffer)
    {
        http_response_result_t result;

        bool failed = false;

        esp_http_client_config_t config = {
            .url = url,
            .method = method,
            .buffer_size_tx = 4096,
            .crt_bundle_attach = esp_crt_bundle_attach,
        };

        esp_http_client_handle_t client = esp_http_client_init(&config);

        esp_http_client_set_header(client, "Content-Type", content_type);
        esp_http_client_set_post_field(client, data, strlen(data));

        esp_err_t err;

        if ((err = esp_http_client_open(client, strlen(data))) != ESP_OK) {
            failed = true;
        }

        int status_code = -1;
        int content_length = -1;

        if (!failed)
        {
            esp_http_client_write(client, data, strlen(data));
            result.content_length = esp_http_client_fetch_headers(client);
            result.status_code = esp_http_client_get_status_code(client);
        }

        //### read...

        if (!failed)
        {
            result.read_len = esp_http_client_read(client, buffer, HTTP_MAX_RECV_BUFFER_SIZE);
        }

        esp_http_client_close(client);
        esp_http_client_cleanup(client);

        result.failed = failed;

        return result;
    }
};

/*bool http_print_result_errors(const char* sub_tag, http_response_result_t r)
{
    if (r.failed) {
        ESP_LOGE(TAG, "[%s] >>> Failed to open HTTP connection!", sub_tag);
        return true;
    }

    if (r.status_code != 200)
    {
        ESP_LOGE(TAG, "[%s] >>> Response failed with code %d", sub_tag, r.status_code);
        return true;
    }

    return false;
}*/
