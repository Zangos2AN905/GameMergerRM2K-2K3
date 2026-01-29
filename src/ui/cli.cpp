#include "cli.hpp"
#include <iostream>
#include <fmt/color.h>

namespace UI {

    void show_banner() {
        // A nice ANSI-colored ASCII logo
        // Using raw string literal for the art
        auto logo = R"(
  _____  __  __ ___   _  __  ___                   
 |  __ \|  \/  |__ \ | |/ / / G \                  
 | |__) | \  / |  ) || ' / | M a | ___ _ __ __ _  ___ _ __ 
 |  _  /| |\/| | / / |  <  |  m e |/ _ \ '__/ _` |/ _ \ '__|
 | | \ \| |  | |/ /_ | . \ |  e r |  __/ | | (_| |  __/ |   
 |_|  \_\_|  |_|____||_|\_\ \___/ \___|_|  \__, |\___|_|   
                                            __/ |          
                                           |___/           
)";
        
        // Print in Cyan for a generic "tech/cool" look, or Magenta for "Chaotic"
        fmt::print(fmt::fg(fmt::color::cyan) | fmt::emphasis::bold, "{}\n", logo);
        fmt::print(fmt::fg(fmt::color::white), "      v0.1 - The Chaotic Merger\n\n");
    }

    void print_info(const std::string& msg) {
        fmt::print(fmt::fg(fmt::color::blue), "[INFO] ");
        fmt::print("{}\n", msg);
    }

    void print_error(const std::string& msg) {
        fmt::print(fmt::fg(fmt::color::red) | fmt::emphasis::bold, "[ERROR] ");
        fmt::print("{}\n", msg);
    }

    void print_success(const std::string& msg) {
        fmt::print(fmt::fg(fmt::color::green), "[SUCCESS] ");
        fmt::print("{}\n", msg);
    }
}
