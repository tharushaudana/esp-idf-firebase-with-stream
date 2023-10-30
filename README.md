# Firebase C++ Library for ESP-IDF (with Stream support)

[![License](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v4.4%2B-brightgreen)](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/)

This C++ Firebase library for ESP-IDF is a reliable and non-blocking solution designed for IoT development on the ESP32 platform. It provides seamless support for streaming data to Firebase and offers various features to enhance your IoT projects.

## Features

- **Non-blocking:** Our library is designed to be non-blocking, allowing your ESP32 to perform other tasks while interacting with Firebase. This is crucial for building responsive IoT applications.

- **Stream Support:** You can easily stream data to Firebase, making it perfect for real-time IoT projects.

- **Improved Reliability:** To ensure a more reliable connection, we recommend setting `CONFIG_LWIP_TCP_MSL` to 10000 ms in your ESP-IDF configuration. This extended Maximum Segment Lifetime (MSL) can help in situations where your network may experience temporary disruptions.

## Getting Started

To start using this C++ library in your ESP-IDF project, follow these steps:

1. Clone this repository or add it as a submodule to your project.

2. Include the necessary header files in your project:

    ```cpp
    #include "firebase-with-stream.h"

    firebase_with_stream firebase;

    firebase_config_t fb_config;
    firebase_credentials_t fb_credentials;
    ```

3. Configure your Firebase credentials and initialize the library:

    ```cpp
    fb_config.api_key = "AIzaSyC2KRrFHQpntl4lMpuh-Wql0TB4njcBIlU";
    fb_config.db_url = "https://greenhouse-project-bec1e-default-rtdb.firebaseio.com";

    fb_credentials.email = "tharusha.udana529@gmail.com";
    fb_credentials.password = "tha2003";

    firebase.set_config(fb_config);
    firebase.set_credentials(fb_credentials);

    firebase.begin();
    ```

4. Start using the library to send and receive data to/from Firebase.

## Example

Here's a simple C++ example of how to update data in Firebase:

```cpp
std::string json_data = 
                "{"
                "\"k1\":\"v22\","
                "\"k2\":\"v23\","
                "\"k3\":\"v24\""
                "}";

firebase.crud.update("/test/o2/.json", json_data.c_str());
