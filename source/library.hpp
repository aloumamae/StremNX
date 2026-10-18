#pragma once

#include <string>
#include <vector>

#include "stremio.hpp"

// Titles saved to "My Library". Mirrors the account library when logged in; otherwise it is
// kept only on the SD card (sdmc:/switch/stremnx/library.json).
class Library {
public:
    void load();
    void save() const;

    bool contains(const std::string& id) const;
    void add(const MetaItem& m);
    void remove(const std::string& id);
    void replace(const std::vector<MetaItem>& items);
    const std::vector<MetaItem>& items() const { return list; }

private:
    std::vector<MetaItem> list;  // newest first
};
