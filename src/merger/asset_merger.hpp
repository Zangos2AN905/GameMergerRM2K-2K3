#pragma once
#include <string>
#include <vector>

class AssetMerger {
public:
    AssetMerger(const std::vector<std::string>& source_dirs);

    // Merges all standard asset folders
    void merge_assets(const std::string& output_dir);

private:
    std::vector<std::string> source_dirs_;
    const std::vector<std::string> asset_folders_ = {
        "Backdrop", "Battle", "Battle2", "BattleChar", "BattleWeapon",
        "CharSet", "ChipSet", "FaceSet", "Frame", "GameOver",
        "Monster", "Movie", "Music", "Panorama", "Picture",
        "Sound", "System", "Title"
    };
};
