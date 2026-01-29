#include "map_merger.hpp"
#include "../ui/cli.hpp"
#include <filesystem>
#include <random>
#include <algorithm>
#include <iostream>
#include <set>

namespace fs = std::filesystem;

MapMerger::MapMerger(const std::vector<std::string>& source_dirs)
    : source_dirs_(source_dirs) {
    
    // Find the source with the most maps to use as base
    size_t max_maps = 0;
    for (size_t i = 0; i < source_dirs_.size(); ++i) {
        size_t count = count_maps(source_dirs_[i]);
        if (count > max_maps) {
            max_maps = count;
            base_index_ = i;
        }
    }
    
    if (!source_dirs_.empty()) {
        UI::print_info("Selected base game: " + source_dirs_[base_index_] + " (" + std::to_string(max_maps) + " maps)");
    }
}

size_t MapMerger::count_maps(const std::string& dir) {
    size_t count = 0;
    try {
        if (!fs::exists(dir)) return 0;
        
        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
                std::string name = entry.path().filename().string();
                std::string lower_name = name;
                std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
                
                if (lower_name.rfind("map", 0) == 0 && lower_name.rfind(".lmu") != std::string::npos) {
                    count++;
                }
            }
        }
    } catch (...) {
        return 0;
    }
    return count;
}

void MapMerger::merge_maps(const std::string& output_dir) {
    if (source_dirs_.empty()) return;

    UI::print_info("Gathering maps from all sources...");
    
    std::vector<fs::path> map_pool;
    for (const auto& src : source_dirs_) {
        try {
            for (const auto& entry : fs::directory_iterator(src)) {
                if (entry.is_regular_file()) {
                    std::string name = entry.path().filename().string();
                    // Case-insensitive check for MapXXXX.lmu
                    // Simple check for startswith Map and endswith .lmu
                    // Windows is case insensitive but we should be careful.
                    std::string lower_name = name;
                    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
                    
                    if (lower_name.rfind("map", 0) == 0 && lower_name.rfind(".lmu") != std::string::npos) {
                        map_pool.push_back(entry.path());
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            UI::print_error(std::string("Error scanning dir: ") + e.what());
        }
    }

    if (map_pool.empty()) {
        UI::print_error("No maps found to merge!");
        return;
    }

    UI::print_info("Found " + std::to_string(map_pool.size()) + " maps total.");
    UI::print_info("Shuffling maps onto base game structure...");

    // Iterate all sources to find UNIQUE map filenames
    std::set<std::string> unique_map_names;

    for (const auto& src : source_dirs_) {
        try {
            for (const auto& entry : fs::directory_iterator(src)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    std::string lower_name = filename;
                    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);

                    if (lower_name.rfind("map", 0) == 0 && lower_name.rfind(".lmu") != std::string::npos) {
                        unique_map_names.insert(filename); // Keeps casing of first occurrence or just works if case matches
                    }
                }
            }
        } catch (...) {}
    }

    int map_count = 0;
    try {
        for (const auto& filename : unique_map_names) {
             // For every unique map name found across ANY game, generate a merged version
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, map_pool.size() - 1);
            
            fs::path source_map = map_pool[dis(gen)];
            fs::path target_map = fs::path(output_dir) / filename;
            
            fs::copy_file(source_map, target_map, fs::copy_options::overwrite_existing);
            map_count++;
        }
        UI::print_success("Merged " + std::to_string(map_count) + " maps (Union of all sources).");
    } catch (const fs::filesystem_error& e) {
         UI::print_error(std::string("Error merging maps: ") + e.what());
    }
}

void MapMerger::copy_lmt(const std::string& output_dir) {
    if (source_dirs_.empty()) return;
    
    fs::path base_lmt = fs::path(source_dirs_[base_index_]) / "RPG_RT.lmt";
    fs::path target_lmt = fs::path(output_dir) / "RPG_RT.lmt";
    
    if (fs::exists(base_lmt)) {
        try {
            fs::copy_file(base_lmt, target_lmt, fs::copy_options::overwrite_existing);
            UI::print_info("Copied map tree (RPG_RT.lmt).");
        } catch (const fs::filesystem_error& e) {
            UI::print_error(std::string("Error copying lmt: ") + e.what());
        }
    } else {
        UI::print_error("Base RPG_RT.lmt not found!");
    }
}
