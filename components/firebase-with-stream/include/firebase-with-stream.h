#include <string>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_tls.h"
#include "esp_log.h"
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
    std::string refresh_token = "AMf-vBxHihNxVrGHMyK1awN-PNGF8JfHAfLQ1hIuNjJwa4xi2bR10O9O_Q3ux1HzxWGv8tpJKKe5GHfy1zprN_5RZqzp2wUBIK0ddC50HQIO5pqb60b_BDbgKSRf576Z-9wOtXo0VBWR2Hqo0A6Qk1u9qpdlOdlzA9QFvaRwAnaVQajWTx4zzDlwxfJW5rrcd3nf6eo7HW649OoMFDLNnxhI8MVDR4m6nJEz0rm8l0BozHcunSADjTKQ1gC1FRUnFpQ5KfWcmEcv";
    int expires_in = 0;
    //---
    bool is_valid = false;
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
    //event_source_stream_parser eparser("event", "data");

    on_firebase_stream_data_cb_t _cb;

    static void _loop_task(void *param);

    void _run_stream();
public:
    firebase_stream(const char* path, on_firebase_stream_data_cb_t cb);
    ~firebase_stream();

    const char* _path;

    bool is_started = false;

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

    void begin_stream(firebase_stream *stream);
};

