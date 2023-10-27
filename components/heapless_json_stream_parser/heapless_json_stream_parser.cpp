#include "heapless_json_stream_parser.h"

/*###########################################################################*/

json_stream_parser::json_stream_parser(on_json_stream_data_cb_t cb)
{
    _cb_data = cb;
    _use_cb = true;
}

json_stream_parser::json_stream_parser()
{
    _use_cb = false;
}

void json_stream_parser::reset() {
    _act = JSON_ACT_FIND_VAL;
    _ptype = JSON_PTYPE_OBJ;
    _path.clear();
    _key.key.clear();
    _val.val.clear();
}

void json_stream_parser::replace_prefix_path(std::string old_p, std::string new_p) 
{
    _path.replace_prefix_path(old_p, new_p);
}

void json_stream_parser::_notify_data() {
    path = _path.to_str(_key);
    value.val = _val.val;
    value.type = _val.type;

    if (_use_cb)
    {
        _cb_data(path, value);
    }
}

void json_stream_parser::_set_current_ptype(int8_t t) {
    _ptype = t;
}

void json_stream_parser::_step_up_path() {
    _depth++;
    _path.up(_key);
}

void json_stream_parser::_step_down_path() {
    json_key_t lkey = _path.lkey();

    _depth--;

    if (_depth == -1)
    {   
        reset();
        return;
    }

    _path.down();

    //### set data of previous parent

    if (_path.n_keys == 0) {
        _set_current_ptype(JSON_PTYPE_OBJ);
        _sa(JSON_ACT_FIND_KEY);
        _key.key = "";
        return;
    }

    if (!lkey.is_empty() && lkey.type == JSON_KEY_TYPE_ARR) {
        _set_current_ptype(JSON_PTYPE_ARR);
        _sa(JSON_ACT_FIND_VAL);
        _key = lkey;
    } else {
        _set_current_ptype(JSON_PTYPE_OBJ);
        _sa(JSON_ACT_FIND_KEY);
    }
}

void json_stream_parser::_set_arr_key() {
    _key.key = "0";
    _key.type = JSON_KEY_TYPE_ARR;
}

void json_stream_parser::_increase_arr_key() {
    if (_key.type != JSON_KEY_TYPE_ARR) return;
    uint8_t new_i = std::stoi(_key.key) + 1;
    _key.key = std::to_string(new_i);
}

void json_stream_parser::_clear_current_key() {
    _key.key = "";
    _key.type = JSON_KEY_TYPE_OBJ;
}

void json_stream_parser::_clear_current_val(int8_t t) {
    _val.val = "";
    _val.type = t;
}

void json_stream_parser::_append_current_key(char c) {
    _key.key += c;
}

void json_stream_parser::_append_current_val(char c) {
    _val.val += c;
}

bool json_stream_parser::_find_value(char c)
{
    if (c == '"') 
    {
        _clear_current_val(JSON_VAL_TYPE_STRING);
    }
    else if (isdigit(c))
    {
        _clear_current_val(JSON_VAL_TYPE_DECIMAL);
        _append_current_val(c);
    }
    else if (c == 't' || c == 'f')
    {
        _clear_current_val(JSON_VAL_TYPE_BOOL);
        _append_current_val(c);
    } 
    else 
    {
        return false;
    }

    _sa(JSON_ACT_READ_VAL);

    return true;
}

bool json_stream_parser::_read_value(char c)
{
    if (_val.type == JSON_VAL_TYPE_STRING) 
    {
        if (c == '"')
        {
            return true;
        }
        else 
        {
            _append_current_val(c);
            return false;
        }
    }
    else if (_val.type == JSON_VAL_TYPE_DECIMAL || _val.type == JSON_VAL_TYPE_FLOAT)
    {
        if (c != '.' && !isdigit(c))
        {
            return true;
        }
        else {
            _append_current_val(c);
            if (c == '.') _val.type = JSON_VAL_TYPE_FLOAT;
            return false;
        }
    }
    else if (_val.type == JSON_VAL_TYPE_BOOL)
    {
        if (c == 'e')
        {
            _append_current_val('e');
            return true;
        }
        else 
        {
            _append_current_val(c);
            return false;
        }
    }

    return false;
}

void json_stream_parser::_sa(int8_t a) {
    _act = a;
}

bool json_stream_parser::parse(char c)
{
    if (_act == JSON_ACT_FIND_VAL)
    {
        if (c == '{')
        {
            _set_current_ptype(JSON_PTYPE_OBJ);
            _sa(JSON_ACT_FIND_KEY);
            _step_up_path();
            return false;
        }
        else if (c == '[')
        {
            _step_up_path();
            _set_current_ptype(JSON_PTYPE_ARR);
            _sa(JSON_ACT_FIND_VAL);
            _set_arr_key();
            return false;
        }
        else if (c == ']')
        {
            _step_down_path();
            return false;
        }
    }

    if (_act == JSON_ACT_FIND_KEY && c == '}')
    {
        _step_down_path();
        return false;
    }
    
    if  (_act == JSON_ACT_FIND_KEY && c == '"') 
    {
        _clear_current_key();
        _sa(JSON_ACT_READ_KEY);
        return false;
    }

    if (_act == JSON_ACT_READ_KEY) {
        if (c == '"') 
        {
            _sa(JSON_ACT_FIND_COL);
        }
        else 
        {   
            _append_current_key(c);
        }

        return false;
    }

    if (_act == JSON_ACT_FIND_COL && c == ':') 
    {
        _sa(JSON_ACT_FIND_VAL);
        return false;
    }

    if (_act == JSON_ACT_FIND_VAL) 
    {
        if (_ptype == JSON_PTYPE_ARR && c == ',') 
        {
            _increase_arr_key();
            return false;
        }
        
        _find_value(c);

        return false;
    }

    if (_act == JSON_ACT_READ_VAL) 
    {
        if (_read_value(c)) 
        {
            if (_ptype == JSON_PTYPE_OBJ)
                _sa(JSON_ACT_FIND_KEY);
            else if (_ptype == JSON_PTYPE_ARR)
                _sa(JSON_ACT_FIND_VAL);

            _notify_data();

            //########### (these are for handle for numeric values)
            if (c == '}' || c == ']')
            {
                _step_down_path();       
            } 
            else if (c == ',' && _ptype == JSON_PTYPE_ARR) 
            {
                _increase_arr_key();
            }   
            //###########

            return true;
        }

        return false;
    }

    return false;
}