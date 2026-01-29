#pragma once
#include <string>

namespace UI {
    void show_banner();
    void print_info(const std::string& msg);
    void print_error(const std::string& msg);
    void print_success(const std::string& msg);
}
