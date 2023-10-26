#include <string>
#include <stdlib.h>

const uint16_t HTTP_MAX_RECV_BUFFER_SIZE = 2048;

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
};

/*
======================================================
CLASS : FIREBASE AUTH
======================================================
*/

class _firebase_auth
{
private:
    bool _sign_in_required = true;

    static void _loop_task(void *param);

    void _sign_in();
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

    void set_config(firebase_config_t c);
    void set_credentials(firebase_credentials_t c);

    void set_wifi_connected(bool b);

    void begin();
};

