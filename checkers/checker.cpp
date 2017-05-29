#include <iostream>
#include <cstdarg>
#include <unordered_set>

#include <execinfo.h>
#include <unistd.h>

#include <boost/functional/hash.hpp>

size_t get_hash_of_string(const std::string& str)
{
    boost::hash<std::string> string_hash;
    return string_hash(str);
}

size_t get_path_hash(std::string module_name, char** path, int size)
{
    auto postfix_pos = module_name.find_first_of('.');
    if (postfix_pos != std::string::npos) {
        module_name = module_name.substr(0, postfix_pos);
        module_name += '(';
    }
    size_t hash = 0;
    // start from second function, as the first will be check and the second will be get_path_hash
    for (unsigned i = 1; i < size; ++i) {
        std::string call_str = path[i];
        if (call_str.find(module_name) == std::string::npos) {
            break;
        }
        auto first_par = call_str.find_first_of('(');
        if (first_par == std::string::npos) {
            continue;
        }
        auto offset_pos = call_str.find_first_of('+');
        if (offset_pos == std::string::npos) {
            continue;
        }
        const std::string& function = call_str.substr(first_par + 1, offset_pos - first_par - 1);
        hash ^= get_hash_of_string(function);
        //std::hash<std::string>()(function);
    }
    printf("call Hash %lu \n", hash);
    return hash;
}

extern "C" {
int check(char* module_name, int count, ...) {
    void *array[100];
    size_t size;
    size = backtrace(array, 100);

    char** strings;
    strings = backtrace_symbols(array, size);
    if (strings == nullptr) {
        abort();
    }
    size_t current_hash = get_path_hash(std::string(module_name), strings, size);
    free(strings);
    //backtrace_symbols_fd(array, size, STDOUT_FILENO);

    bool is_valid_path = false;
    va_list args_list;
    va_start(args_list, count);
    for (unsigned i = 0; i < count; ++i) {
        size_t hash = va_arg(args_list, size_t);
        if (current_hash == hash) {
            is_valid_path = true;
            break;
        }
    }
    if (!is_valid_path) {
        printf("Untrusted call path\n");
        abort();
    }
    return 0;
}
}
