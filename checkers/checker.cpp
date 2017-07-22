#include <iostream>
#include <cstdarg>
#include <unordered_set>

#include <execinfo.h>
#include <unistd.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

#include <boost/functional/hash.hpp>
#include <unordered_set>
#include <unordered_map>
#include <chrono>

size_t get_hash_of_string(const std::string& str)
{
    boost::hash<std::string> string_hash;
    return string_hash(str);
}


class call_stack_mgr
{
public:
    static call_stack_mgr& get()
    {
        static call_stack_mgr mgr;
        return mgr;
    }

public:
    size_t get_stack_hash();

private:
    std::unordered_map<unw_word_t, std::string> function_addresses;
};

size_t call_stack_mgr::get_stack_hash()
{
    size_t hash = 0;
    char name[256];
    unw_cursor_t cursor;
    unw_context_t uc;

    unw_getcontext (&uc);
    unw_init_local (&cursor, &uc);

    std::unordered_set<std::string> processed;
    while (unw_step(&cursor) > 0) {
        unw_word_t offset, pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        if (pc == 0) {
            break;
        }
        //printf("0x%lx:\n", pc);
        auto name_pos = function_addresses.find(pc);
        if (name_pos != function_addresses.end()) {
            if (name_pos->second == "check") {
                continue;
            }
            if (!processed.insert(name_pos->second).second) {
                continue;
            }
            hash ^= get_hash_of_string(name_pos->second);
            //printf("use existing name %s\n", name_pos->second.c_str());
            if (name_pos->second == "main") {
                break;
            }
            continue;
        }
        //printf("no existing name. get from stack\n");

        char sym[256];
        if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
            std::string f_name = std::string(sym);
            auto dyninst_pos = f_name.find("_dyninst");
            if (dyninst_pos != std::string::npos) {
                f_name = f_name.substr(0, dyninst_pos);
            }
            function_addresses.insert(std::make_pair(pc, f_name));
            //printf("%s\n", f_name.c_str());
            if (f_name == "check") {
                continue;
            }
            if (!processed.insert(f_name).second) {
                continue;
            }
            hash ^= get_hash_of_string(f_name);
            if (f_name == "main") {
                break;
            }
        }
    }
    return hash;
}

extern "C" {

size_t previous_path_hash;

int check(int count, ...)
{
    call_stack_mgr& mgr = call_stack_mgr::get();
    size_t current_hash = mgr.get_stack_hash();
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
        printf("Untrusted call path. Current hash %zu\n", current_hash);
        abort();
    }
    return 0;
}

}
