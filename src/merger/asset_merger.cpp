#include "asset_merger.hpp"
#include "../ui/cli.hpp"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

AssetMerger::AssetMerger(const std::vector<std::string>& source_dirs)
    : source_dirs_(source_dirs) {
}

void AssetMerger::merge_assets(const std::string& output_dir) {
    UI::print_info("Merging Assets (this may take a while)...");

    int file_count = 0;

    for (const auto& folder : asset_folders_) {
        fs::path target_subdir = fs::path(output_dir) / folder;
        if (!fs::exists(target_subdir)) {
            fs::create_directories(target_subdir);
        }

        for (const auto& src : source_dirs_) {
            fs::path src_subdir = fs::path(src) / folder;
            if (fs::exists(src_subdir) && fs::is_directory(src_subdir)) {
                for (const auto& entry : fs::directory_iterator(src_subdir)) {
                    if (entry.is_regular_file()) {
                        try {
                            // Copy with overwrite
                            fs::copy_file(entry.path(), target_subdir / entry.path().filename(), fs::copy_options::overwrite_existing);
                            file_count++;
                        } catch (const fs::filesystem_error&) {
                            // Ignore specific copy errors (locked files etc)
                        }
                    }
                }
            }
        }
    }
    
    UI::print_success("Merged " + std::to_string(file_count) + " asset files.");
}
