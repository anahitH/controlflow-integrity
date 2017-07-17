#include <iostream>
#include <cstdarg>

#include <execinfo.h>
#include <unistd.h>
#include <libunwind.h>

#include <boost/functional/hash.hpp>
#include <unordered_set>
#include <string>


#define UNW_LOCAL_ONLY

size_t get_hash_of_string(const std::string& str)
{
    boost::hash<std::string> string_hash;
    return string_hash(str);
}

class call_path
{
public:
    call_path();

    size_t compute_hash(std::string module_name, char** path, int size);

private:
    std::vector<std::string> m_path;
};

call_path::call_path()
{
    char name[256];
    unw_cursor_t cursor;
    unw_context_t uc;
    unw_word_t ip, sp, offp;

    unw_getcontext (&uc);
    unw_init_local (&cursor, &uc);

    while (unw_step(&cursor) > 0)
    {
        unw_word_t offset, pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0) {
            break;
        }

        char sym[256];
        if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
            std::string f_name = std::string(sym);
            auto dyninst_pos = f_name.find("_dyninst");
            if (dyninst_pos != std::string::npos) {
                f_name = f_name.substr(0, dyninst_pos);
            }
            m_path.push_back(std::string(sym));
        }
    }
}

size_t call_path::compute_hash(std::string module_name, char** path, int size)
{
    auto postfix_pos = module_name.find_first_of('.');
    if (postfix_pos != std::string::npos) {
        module_name = module_name.substr(0, postfix_pos);
        module_name += '(';
    }
    std::unordered_set<std::string> processed;
    size_t hash = 0;
    // start from second function, as the first will be check and the second will be get_path_hash
    for (unsigned i = 1; i < size; ++i) {
        std::string call_str = path[i];
        if (call_str.find(module_name) == std::string::npos) {
            break;
        }
        std::string function = m_path[i];
        if (!processed.insert(function).second) {
            continue;
        }
        hash ^= get_hash_of_string(function);
        //std::hash<std::string>()(function);
    }
    //printf("call Hash %lu \n", hash);
    return hash;
}

extern "C" {

int check(char* module_name, int count, ...) {
    call_path current_path;
    void *array[100];
    size_t size;
    size = backtrace(array, 100);

    char** strings;
    strings = backtrace_symbols(array, size);
    if (strings == nullptr) {
        abort();
    }
    size_t current_hash = current_path.compute_hash(std::string(module_name), strings, size);
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
