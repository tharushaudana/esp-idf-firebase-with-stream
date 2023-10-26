#include <stdio.h>
#include "esp_http_client.h"
#include "esp_crt_bundle.h"
#include "esp_tls.h"
#include "esp_log.h"
#include "firebase-with-stream.h"

static const char *TAG = "FWS";

firebase_with_stream::firebase_with_stream()
{
    _auth.netstat = &_netstat;
    _auth.config = &_config;
    _auth.credentials = &_credentials;
    _auth.token_data = &_token_data;
}

firebase_with_stream::~firebase_with_stream()
{
}

void firebase_with_stream::set_config(firebase_config_t c) 
{
    _config = c;
}

void firebase_with_stream::set_credentials(firebase_credentials_t c) 
{
    _credentials = c;
}

void firebase_with_stream::set_wifi_connected(bool b) 
{
    _netstat.wifi_connected = b;
}

void firebase_with_stream::begin() 
{
    _auth.init();
}
