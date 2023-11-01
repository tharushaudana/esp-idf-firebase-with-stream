#include <stdio.h>
#include <string>
#include <functional>
#include <iostream>
#include <sstream>
#include <cstring>

const int8_t JSON_PTYPE_OBJ = 0;
const int8_t JSON_PTYPE_ARR = 1;

const int8_t JSON_KEY_TYPE_OBJ = 0;
const int8_t JSON_KEY_TYPE_ARR = 1;
const int8_t JSON_KEY_TYPE_NULL = 2;

const int8_t JSON_VAL_TYPE_NULL = -1;
const int8_t JSON_VAL_TYPE_STRING = 0;
const int8_t JSON_VAL_TYPE_DECIMAL = 1;
const int8_t JSON_VAL_TYPE_FLOAT = 2;
const int8_t JSON_VAL_TYPE_BOOL = 3;

const int8_t JSON_ACT_FIND_KEY = 1;
const int8_t JSON_ACT_READ_KEY = 2;
const int8_t JSON_ACT_FIND_COL = 3;
const int8_t JSON_ACT_FIND_VAL = 4;
const int8_t JSON_ACT_READ_VAL = 5;

struct json_key_t
{
    int8_t type = JSON_KEY_TYPE_OBJ; 
    std::string key = "";

    bool is_empty() 
    {
        return key.length() == 0;
    }

    std::string to_str() 
    {
        if (type == JSON_KEY_TYPE_OBJ)
            return key;
        else 
            return "[" + key + "]";
    }

    void set_from_str(std::string s) 
    {
        const char* str = s.c_str();

        key.clear();

        if (str[0] == '[')
        {
            type = JSON_KEY_TYPE_ARR;

            for (size_t i = 0; i < strlen(str); i++)
            {
                if (str[i] != '[' && str[i] != ']') 
                {
                    key += str[i];
                }
            }
        }
        else 
        {
            key.append(s);
        }
    }
};

struct json_val_t
{
    int8_t type = JSON_KEY_TYPE_NULL; 
    std::string val;

    template <typename T>
    void get_value(T& destination) {
        if (std::is_same<T, bool>::value) 
        {
            destination = val == "true";
        } 
        else {
            std::istringstream ss(val);
            ss >> destination;
        }
    }
};

struct json_pair_t
{
    std::string path;
    json_val_t value;

    bool used = false;

    template <typename T>
    void get_value_if_path(T& destination, const char* p) 
    {
        if (path != p) return;
        value.get_value(destination);
        used = true;
    }
};


/*struct path_t
{
    json_key_t keys[20];
    int n_keys = 0;

    void up(json_key_t k)
    {
        if (k.key.length() == 0 || n_keys >= 20) return;
        keys[n_keys++] = k;
    }

    void down()
    {
        n_keys--;
    }

    json_key_t* lkey()
    {
        if (n_keys == 0) return nullptr;
        return &keys[n_keys - 1];
    }

    std::string k_to_str(json_key_t k) 
    {
        if (k.type == JSON_KEY_TYPE_OBJ)
            return k.key;
        else 
            return "[" + k.key + "]";
    }

    std::string to_str(json_key_t suffix_key)
    {
        std::string str = "";

        for (uint8_t i = 0; i < n_keys; i++)
        {
            json_key_t k = keys[i];

            str += k_to_str(k);

            if (i < n_keys - 1)
            {
                str += "/";
            }
        }

        if (suffix_key.key.length() > 0)
        {
            if (str.length() > 0) str += "/";
            str += k_to_str(suffix_key);
        }

        return str;
    }
};*/


struct path_t
{
    int n_keys = 0;
    int16_t key_ends[20];
    std::string keys = "";

    std::string old_prefix_path_str = "";
    std::string new_prefix_path_str = "";

    void clear()
    {
        n_keys = 0;
        keys.clear();
        old_prefix_path_str.clear();
        new_prefix_path_str.clear();
    }

    void replace_prefix_path(std::string old_p, std::string new_p)
    {
        if (old_p == "/") return;
        old_prefix_path_str = old_p;
        new_prefix_path_str = new_p;
    }

    void up(json_key_t k)
    {
        if (k.key.length() == 0 || n_keys >= 20) return;

        std::string kstr = k.to_str();

        keys.append(kstr);

        if (n_keys > 0) 
        {
            key_ends[n_keys] = kstr.length() + key_ends[n_keys - 1];
        }
        else 
        {
            key_ends[n_keys] = kstr.length() - 1;
        }

        n_keys++;
    }

    void down()
    {
        if (n_keys > 1)
        {
            keys.erase(key_ends[n_keys - 2] + 1);            
        } 
        else if (n_keys == 1)
        {
            keys.erase(0);            
        }
        else 
        {
            return;
        }

        n_keys--;
    }

    json_key_t lkey()
    {
        if (n_keys == 0) 
        {
            json_key_t empty_key;
            return empty_key;
        }

        return key_at(n_keys - 1);
    }

    json_key_t key_at(int16_t i) 
    {
        std::string kstr;
        json_key_t key;

        if (i == 0)
        {
            kstr = keys.substr(0, key_ends[0] + 1);
        }
        else
        {
            kstr = keys.substr(key_ends[i - 1] + 1, key_ends[i] - key_ends[i - 1]);
        }

        key.set_from_str(kstr);

        return key;
    }

    std::string to_str(json_key_t suffix_key)
    {
        std::string str = "";

        bool prefix_replaced = false;

        for (uint8_t i = 0; i < n_keys; i++)
        {
            json_key_t k = key_at(i);

            str += "/";
            str += k.to_str();

            if (!prefix_replaced && old_prefix_path_str.length() > 0)
            {
                if (str == old_prefix_path_str)
                {
                    if (new_prefix_path_str == "/")
                    {
                        str = "";
                    }
                    else 
                    {
                        str = new_prefix_path_str;
                    }

                    prefix_replaced = true;
                }
            }
        }

        if (!suffix_key.is_empty())
        {
            str += "/";
            str += suffix_key.to_str();
        }

        return str;
    }
};

typedef std::function<void(json_pair_t)> on_json_stream_data_cb_t;

class json_stream_parser
{
private:
    on_json_stream_data_cb_t _cb_data;

    // current action
    uint8_t _act = JSON_ACT_FIND_VAL; 
    // current parent type (object or array)
    uint8_t _ptype = JSON_PTYPE_OBJ; 
    // current key
    json_key_t _key; 
    // current value
    json_val_t _val; 
    // current depth
    int8_t _depth = -1; 

    path_t _path;

    bool _use_cb = false;

    bool _is_resetted = false;

    void _notify_data();

    void _set_current_ptype(int8_t t);
    void _step_up_path();
    void _step_down_path();
    void _set_arr_key();
    void _increase_arr_key();
    void _clear_current_key();
    void _clear_current_val(int8_t t);
    void _append_current_key(char c);
    void _append_current_val(char c);
    void _sa(int8_t a);
    bool _find_value(char c);
    bool _read_value(char c);
public:
    json_stream_parser(on_json_stream_data_cb_t cb);
    json_stream_parser();

    // these are when not using callback function
    json_pair_t pair;

    bool parse(char c);
    void replace_prefix_path(std::string old_p, std::string new_p);
    void reset();
    bool is_resetted();
};

