#include <stdio.h>
#include <string>
#include <functional>
#include <iostream>
#include <sstream>

const int8_t EVTS_ACT_FIND_EVENT = 0;
const int8_t EVTS_ACT_READ_EVENT = 1;
const int8_t EVTS_ACT_WAIT_FOR_EVENT_LINE_END = 2;
const int8_t EVTS_ACT_FIND_DATA = 3;
const int8_t EVTS_ACT_READ_DATA = 4;
const int8_t EVTS_ACT_WAIT_FOR_DATA_LINE_END = 5;

typedef std::function<void(char)> on_evts_data_char_cb_t;
typedef std::function<on_evts_data_char_cb_t(std::string)> on_evts_stream_data_cb_t;
//typedef std::function<void(std::string, char c)> on_evts_stream_data_cb_t;

class event_source_stream_parser
{
private:
    on_evts_stream_data_cb_t _cb_stream;
    on_evts_data_char_cb_t _cb_char;

    int8_t _act = EVTS_ACT_FIND_EVENT;

    std::string _event_key;
    std::string _data_key;

    std::string _current_event_key;
    std::string _current_data_key;

    std::string _current_event;

    bool _use_cb = false;

    void _notify_event();
    void _notify_data_char(char c);

    void _clear_current_event_key();
    void _clear_current_data_key();
    void _append_current_event_key(char c);
    void _append_current_data_key(char c);
    void _clear_current_event();
    void _append_current_event(char c);
    bool _is_match_event_key();
    bool _is_match_data_key();
    void _sa(int8_t a);
public:
    event_source_stream_parser(std::string event_key, std::string data_key, on_evts_stream_data_cb_t cb);
    event_source_stream_parser(std::string event_key, std::string data_key);

    std::string event;
    char data;

    bool parse(char c);
};
