#include "event_source_stream_parser.h"

event_source_stream_parser::event_source_stream_parser(std::string event_key, std::string data_key, on_evts_stream_data_cb_t cb)
{
    _cb_stream = cb;
    _event_key = event_key;
    _data_key = data_key;
    _use_cb = true;
}

event_source_stream_parser::event_source_stream_parser(std::string event_key, std::string data_key)
{
    _event_key = event_key;
    _data_key = data_key;
    _use_cb = false;
}

bool event_source_stream_parser::event_name_parsed() 
{
    if (!_event_name_parsed) return false;
    _event_name_parsed = false;
    return true;
}

void event_source_stream_parser::_notify_event() 
{
    event = _current_event;
    _event_name_parsed = true;
    if (!_use_cb) return;
    _cb_char = _cb_stream(_current_event);
}

void event_source_stream_parser::_notify_data_char(char c) 
{
    data = c;
    if (_use_cb) _cb_char(c);
}

void event_source_stream_parser::_clear_current_event_key() 
{
    _current_event_key = "";
}

void event_source_stream_parser::_clear_current_data_key() 
{
    _current_data_key = "";
}

void event_source_stream_parser::_clear_current_event() 
{
    _current_event = "";
}

void event_source_stream_parser::_append_current_event_key(char c) 
{
    _current_event_key += c;
}

void event_source_stream_parser::_append_current_data_key(char c) 
{
    _current_data_key += c;
}

void event_source_stream_parser::_append_current_event(char c) 
{
    _current_event += c;
}

bool event_source_stream_parser::_is_match_event_key() 
{
    return _current_event_key == _event_key;
}

bool event_source_stream_parser::_is_match_data_key() 
{
    return _current_data_key == _data_key;
}

void event_source_stream_parser::_sa(int8_t a) 
{
    _act = a;
}

bool event_source_stream_parser::parse(char c) 
{
    if (_act == EVTS_ACT_FIND_EVENT && c != ' ' && c != '\n')
    {
        if (c == ':') 
        {
            if (_is_match_event_key()) 
            {
                _clear_current_event();
                _sa(EVTS_ACT_READ_EVENT);
            }
            _clear_current_event_key();
        } 
        else 
        {
            _append_current_event_key(c);
        }

        return false;
    }

    if (_act == EVTS_ACT_READ_EVENT && c != ' ')
    {
        if (c == '\n') 
        {
            _notify_event();
            _sa(EVTS_ACT_FIND_DATA);
        } 
        else 
        {
            _append_current_event(c);
        }

        return false;
    }

    if (_act == EVTS_ACT_FIND_DATA && c != ' ' && c != '\n')
    {
        if (c == ':') 
        {
            if (_is_match_data_key()) 
            {
                _sa(EVTS_ACT_READ_DATA);
            } 
            else 
            {
                _sa(EVTS_ACT_FIND_EVENT);
            }
            _clear_current_data_key();
        } 
        else 
        {
            _append_current_data_key(c);
        }

        return false;
    }

    if (_act == EVTS_ACT_READ_DATA && c != ' ')
    {
        if (c == '\n') 
        {
            _sa(EVTS_ACT_FIND_EVENT);
        } 
        else 
        {
            _notify_data_char(c);
            return true;
        }
        
        return false;
    }

    return false;
}
