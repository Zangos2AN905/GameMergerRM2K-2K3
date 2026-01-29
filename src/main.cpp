#include <iostream>
#include <string>
#include <vector>
#include "ui/cli.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>

#include "ui/cli.hpp"
#include "ui/native_ui.hpp"
#include "merger/ldb_merger.hpp"
#include "merger/map_merger.hpp"
#include "merger/asset_merger.hpp"

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

namespace fs = std::filesystem;

void setup_console() {
    #ifdef _WIN32
    // Enable Colors
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(hOut, dwMode);
    }
    // Init COM for the thread
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    #endif
}

std::string get_input(const std::string& prompt) {
    std::cout << "\033[1;36m[?] " << prompt << "\033[0m "; // Cyan prompt
    std::string line;
    std::getline(std::cin, line);
    return line;
}

int main(int argc, char* argv[]) {
    setup_console();
    UI::show_banner();

    std::vector<std::string> sources;
    std::string output_path;

    if (argc > 1) {
        // TODO: Proper Arg parsing
        // For now, assume just interactive mode is primary
        UI::print_info("Arguments detected, but interactive mode is currently default.");
    }

    // Interactive Mode
    UI::print_info("Lets create a masterpiece.");
    
    // 1. Get Base Game
    UI::print_info("Please select the BASE GAME folder (The 'skeleton').");
    while (true) {
        std::string path = UI::select_folder("Select Base Game Folder");
        if (path.empty()) {
             // If user cancelled dialog, ask if they want to type it or exit
             std::string choice = get_input("Dialog cancelled. Type path manually? (y/n/exit):");
             if (choice == "exit") return 0;
             if (choice == "y" || choice == "Y") {
                 path = get_input("Path:");
             }
        }

        if (!path.empty() && fs::exists(path) && fs::is_directory(path)) {
            sources.push_back(path);
            UI::print_success("Selected: " + path);
            break;
        }
        UI::print_error("Invalid directory. Please try again.");
    }

    // 2. Get Additional Games
    while (true) {
        UI::print_info("Select another game to mix in?");
        std::string choice = get_input("Press ENTER to open dialog, or type 'done' to finish:");
        
        if (choice == "done") {
             if (sources.size() < 1) { 
                // Should not happen as we enforce base game
                break;
            } else if (sources.size() == 1) {
                std::string confirm = get_input("Only 1 game selected. Merge will just copy. Proceed? (y/n):");
                if (confirm == "y" || confirm == "Y") break;
            } else {
                break;
            }
        } else {
            std::string path = UI::select_folder("Select Additional Game Folder");
            if (!path.empty() && fs::exists(path) && fs::is_directory(path)) {
                sources.push_back(path);
                UI::print_success("Added source: " + path);
            } else {
                if (!path.empty()) UI::print_error("Directory does not exist.");
                else UI::print_info("Cancelled.");
            }
        }
    }

    // 3. Output
    UI::print_info("Select where to save the CHAOS.");
    while (true) {
        output_path = UI::select_folder("Select Output Directory");
        if (output_path.empty()) {
             std::string choice = get_input("Dialog cancelled. Type path manually? (y/n):");
             if (choice == "y" || choice == "Y") {
                 output_path = get_input("Output Path:");
             }
        }
        
        if (!output_path.empty()) break;
    }

    UI::print_info("Starting Merge Process...");
    UI::print_info("Sources: " + std::to_string(sources.size()));
    UI::print_info("Output: " + output_path);

    try {
        // Create Output Dir
        if (!fs::exists(output_path)) {
            fs::create_directories(output_path);
        }

        // 1. Assets
        AssetMerger assets(sources);
        assets.merge_assets(output_path);

        // 2. Maps
        MapMerger maps(sources);
        maps.merge_maps(output_path);
        maps.copy_lmt(output_path);

        // 3. Database
        LdbMerger ldb(sources);
        ldb.merge(output_path);

        UI::show_banner();
        UI::print_success("CHAOS COMPLETE. Your new game awaits in: " + output_path);
        UI::print_info("Don't forget to install the RTP if needed!");

    } catch (const std::exception& e) {
        UI::print_error(std::string("Create Failure: ") + e.what());
    }

    UI::print_info("Press Enter to exit...");
    std::cin.get();

    return 0;
}
