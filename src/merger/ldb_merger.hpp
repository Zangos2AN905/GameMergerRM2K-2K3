#pragma once
#include <string>
#include <vector>
#include <lcf/rpg/database.h>

class LdbMerger {
public:
    LdbMerger(const std::vector<std::string>& sources);
    void merge(const std::string& output_path);

private:
    std::vector<std::string> sources_;
    
    // Helper to mix specific dictionary types (actors, items, etc.)
    // We implement this as a template or generic function in cpp usually, 
    // but for simplicity we'll have specific methods or a generic internal one.
    void mix_database_content(lcf::rpg::Database& target_db, const std::vector<lcf::rpg::Database>& source_dbs);
};
