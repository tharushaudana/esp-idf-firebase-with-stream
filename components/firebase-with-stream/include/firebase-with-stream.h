#include <string>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_tls.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "heapless_json_stream_parser.h"
#include "event_source_stream_parser.h"
#include "http_request.h"

typedef struct firebase_netstat_t
{
    bool wifi_connected = false;

    bool is_ready()
    {
        return wifi_connected;
    }
};

typedef struct firebase_config_t
{
    std::string api_key = "";
    std::string db_url = "";
};

typedef struct firebase_credentials_t
{
    std::string email = "";
    std::string password = "";
};

typedef struct firebase_token_data_t
{
    std::string id_token = "";
    std::string refresh_token = "";
    int expires_in = 0;
    //---
    bool is_valid = false;

    void check(int descrease_by)
    {
        if (expires_in <= 0) return;

        expires_in -= descrease_by;

        ESP_LOGW("DDD", "expires in: %d", expires_in);

        if (expires_in <= 0)
        {
            is_valid = false;
            ESP_LOGW("FWS:TOKEN", "Expired.");
        }        
    }

    void set_valid(bool b)
    {
        if (b) 
        {
            ESP_LOGI("DDD", "expires_in: %d", expires_in);
        }

        is_valid = b;
    }
};

/*
======================================================
CLASS : FIREBASE AUTH
======================================================
*/

class _firebase_auth
{
private:
    json_stream_parser jparser;

    bool _sign_in_required = true;

    static void _loop_task(void *param);

    void _sign_in();
    void _token_refresh();
    void _cycle();
public:
    _firebase_auth();
    ~_firebase_auth();

    firebase_netstat_t *netstat;
    firebase_config_t *config;
    firebase_credentials_t *credentials;
    firebase_token_data_t *token_data;

    void init();
};

/*
======================================================
CLASS : FIREBASE CRUD
======================================================
*/

class _firebase_crud
{
private:
    json_stream_parser jparser;
public:
    _firebase_crud();
    ~_firebase_crud();

    firebase_netstat_t *netstat;
    firebase_config_t *config;
    firebase_token_data_t *token_data;

    bool update(const char* path, const char *json_data);
};

/*
======================================================
CLASS : FIREBASE STREAM
======================================================
*/

typedef std::function<void(char c)> on_firebase_stream_data_cb_t;

class firebase_stream
{
private:
    on_firebase_stream_data_cb_t _cb;

    event_source_stream_parser eparser;

    bool _first_data_received = false;

    int64_t _wait_for_first_data_micros = 10000000; // 10 seconds
    int64_t _timeout_micros = 35000000; // 35 seconds

    int64_t _opened_at = 0;
    int64_t _last_data_at = 0;

    static void _loop_task(void *param);

    void _opened();
    bool _wait_for_first_data();
    void _notify_first_data_received();

    void _check_timeout();
    void _extend_timeout();

    void _run_stream();
public:
    firebase_stream(on_firebase_stream_data_cb_t cb);
    ~firebase_stream(); 

    const char* path;

    bool is_started = false;
    bool is_timedout = false;

    firebase_netstat_t *netstat;
    firebase_config_t *config;
    firebase_token_data_t *token_data;

    void start();
    void cancel();
};

/*
======================================================
CLASS : FIREBASE WITH STREAM (MAIN)
======================================================
*/

class firebase_with_stream
{
private:
    firebase_netstat_t _netstat;
    firebase_config_t _config;
    firebase_credentials_t _credentials;
    firebase_token_data_t _token_data;

    _firebase_auth _auth;
public:
    firebase_with_stream();
    ~firebase_with_stream();

    _firebase_crud crud;

    void set_config(firebase_config_t c);
    void set_credentials(firebase_credentials_t c);

    void set_wifi_connected(bool b);

    void begin();

    void begin_stream(firebase_stream *stream, const char* path);
};

