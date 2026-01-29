#include "ldb_merger.hpp"
#include "../ui/cli.hpp"
#include <lcf/ldb/reader.h>
#include <random>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

LdbMerger::LdbMerger(const std::vector<std::string>& sources) 
    : sources_(sources) {
}

// Helper to pick a random item from a list of vectors
template <typename T>
const T& pick_random(const std::vector<std::vector<T>>& pools, int index) {
    // Collect candidates that have this index
    // Note: LibLCF vectors usually map index to ID, but sometimes they are just lists.
    // In RPG Maker, ID often equals index+1 or is explicit.
    // For simplicity, we assume vectors are index-aligned or we just pick from available pools.
    
    // Actually, LibLCF structs like `actors` are std::vector<lcf::rpg::Actor>.
    // We want to pick the `i`-th actor from a random source that HAS an `i`-th actor.
    
    std::vector<const T*> candidates;
    for (const auto& pool : pools) {
        if (index < pool.size()) {
            candidates.push_back(&pool[index]);
        }
    }
    
    if (candidates.empty()) {
        throw std::runtime_error("No candidates found for index " + std::to_string(index));
    }
    
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, candidates.size() - 1);
    
    return *candidates[dis(gen)];
}

// Generic mixer for simple vectors (Actors, Items, etc.)
// Generic mixer for simple vectors (Actors, Items, etc.)
template <typename T>
void mix_vector_field(std::vector<T>& target_vec, const std::vector<lcf::rpg::Database>& source_dbs, const std::vector<T> lcf::rpg::Database::* field) {
    // 1. Determine Max Size across all sources
    size_t max_size = target_vec.size(); // Start with base size
    for (const auto& db : source_dbs) {
        max_size = std::max(max_size, (db.*field).size());
    }
    
    // 2. Resize target to Max, filling with default constructed items (or copy of last?)
    // LibLCF usually expects valid IDs. If we just resize, the new items have ID=0.
    // We should probably rely on valid picks to fill them.
    if (target_vec.size() < max_size) {
        // We create empty slots then fill them.
        // NOTE: LibLCF vector items usually need ID matching index+1.
        size_t old_size = target_vec.size();
        target_vec.resize(max_size);
        for (size_t i = old_size; i < max_size; ++i) {
            target_vec[i].ID = static_cast<int>(i + 1);
        }
    }

    // Pre-assemble pools
    std::vector<std::vector<T>> pools;
    for (const auto& db : source_dbs) {
        pools.push_back(db.*field);
    }

    // 3. Mix
    for (size_t i = 0; i < target_vec.size(); ++i) {
        int original_id = target_vec[i].ID;
        
        try {
            // This now might pick from a source that DOES have index i, even if base didn't.
            const T& chosen = pick_random(pools, static_cast<int>(i));
            target_vec[i] = chosen;
            target_vec[i].ID = original_id; 
        } catch (...) {
            // No source has this index? This happens if we resized to Max but "pick_random" fails?
            // "pick_random" collects candidates where index < pool.size().
            // Since max_size is determined from pools, at least one pool MUST have this index.
            // So this catch should technically not trigger unless empty pools.
            
            // If it fails, we have a blank object with correct ID.
        }
    }
}

void LdbMerger::merge(const std::string& output_path) {
    if (sources_.empty()) {
        UI::print_error("No sources provided for LDB merge.");
        return;
    }

    UI::print_info("Reading source databases...");
    std::vector<std::unique_ptr<lcf::rpg::Database>> dbs;

    // Load main database (first source) to serve as the base
    std::string base_path = (fs::path(sources_[0]) / "RPG_RT.ldb").string();
    auto base_db = lcf::LDB_Reader::Load(base_path, ""); 
    
    if (!base_db) {
        UI::print_error("Failed to load base LDB: " + base_path);
        return;
    }

    // Load others
    std::vector<lcf::rpg::Database> source_db_objects;
    for (const auto& src : sources_) {
        std::string path = (fs::path(src) / "RPG_RT.ldb").string();
        auto db = lcf::LDB_Reader::Load(path, "");
        if (db) {
            UI::print_info("Loaded: " + path);
            source_db_objects.push_back(*db);
        } else {
            UI::print_error("Failed to load or skip: " + path);
        }
    }

    if (source_db_objects.empty()) {
         UI::print_error("No valid databases loaded.");
         return;
    }

    UI::print_info("Mixing database content...");
    
    // Mix Actors
    UI::print_info("  - Actors");
    mix_vector_field(base_db->actors, source_db_objects, &lcf::rpg::Database::actors);
    
    // Mix Skills
    UI::print_info("  - Skills");
    mix_vector_field(base_db->skills, source_db_objects, &lcf::rpg::Database::skills);
    
    // Mix Items
    UI::print_info("  - Items");
    mix_vector_field(base_db->items, source_db_objects, &lcf::rpg::Database::items);
    
    // Mix Enemies
    UI::print_info("  - Enemies");
    mix_vector_field(base_db->enemies, source_db_objects, &lcf::rpg::Database::enemies);
    
    // Mix States
    UI::print_info("  - States");
    mix_vector_field(base_db->states, source_db_objects, &lcf::rpg::Database::states);

    // Mix Animations
    UI::print_info("  - Animations (This might be chaotic!)");
    mix_vector_field(base_db->animations, source_db_objects, &lcf::rpg::Database::animations);

    // Additional fields to prevent crashes due to missing refs
    UI::print_info("  - Classes");
    mix_vector_field(base_db->classes, source_db_objects, &lcf::rpg::Database::classes);

    UI::print_info("  - Common Events");
    mix_vector_field(base_db->commonevents, source_db_objects, &lcf::rpg::Database::commonevents);

    UI::print_info("  - Terrains");
    mix_vector_field(base_db->terrains, source_db_objects, &lcf::rpg::Database::terrains);

    UI::print_info("  - Attributes");
    mix_vector_field(base_db->attributes, source_db_objects, &lcf::rpg::Database::attributes);

    // Write output
    std::string out_file = (fs::path(output_path) / "RPG_RT.ldb").string();
    UI::print_info("Writing merged LDB to: " + out_file);
    
    // Ensure output directory exists
    fs::create_directories(output_path);

    bool success = lcf::LDB_Reader::Save(out_file, *base_db, "");
    if (success) {
        UI::print_success("Database merged successfully!");
    } else {
        UI::print_error("Failed to write database to " + out_file);
    }
}
