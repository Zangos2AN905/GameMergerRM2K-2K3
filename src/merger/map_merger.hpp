#pragma once
#include <string>
#include <vector>

class MapMerger {
public:
    MapMerger(const std::vector<std::string>& source_dirs);
    
    // Merges maps by shuffling them into the output directory
    // based on the first source's map IDs.
    void merge_maps(const std::string& output_dir);

    // Copies the Map Tree (RPG_RT.lmt) from the first source
    void copy_lmt(const std::string& output_dir);

private:
    std::vector<std::string> source_dirs_;
    size_t base_index_ = 0;

    // Helper to count maps in a directory
    size_t count_maps(const std::string& dir);
};
