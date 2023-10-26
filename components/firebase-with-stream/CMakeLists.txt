idf_component_register(SRCS "firebase-with-stream.cpp" "_auth.cpp"
                    INCLUDE_DIRS "include"
                    PRIV_REQUIRES esp_http_client esp-tls mbedtls
                    REQUIRES heapless_json_stream_parser event_source_stream_parser)
