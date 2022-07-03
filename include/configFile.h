#pragma once

#include <filesystem>

#include "json.hpp"

// Saving data: 1) Create configFile; 2) insert data; 3) save()
// Loading data: 1) Create configFile; 2) load(); 3) get data;

class ConfigFile {
using json = nlohmann::json;

public:
    ConfigFile(const std::filesystem::path& configpath);
    ~ConfigFile(void) = default;

    void load(void);
    void save(void);

    // TODO: assure all types allow for const-ref and change insert function
    template <typename TP>
    void insert(const TP& value);

    template <typename TP>
    TP get(void);

private:
    json data;
    std::filesystem::path configpath;
};
