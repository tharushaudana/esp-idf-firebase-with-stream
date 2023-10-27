idf_component_register(SRCS "_crud.cpp" "firebase-with-stream.cpp" "_auth.cpp" "_crud.cpp" "_stream.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_http_client esp-tls mbedtls
                    REQUIRES heapless_json_stream_parser event_source_stream_parser)
