#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

// Bencode (BitTorrent's serialization format).
struct BValue {
    enum Type { None, Int, Str, List, Dict } type = None;
    int64_t i = 0;
    std::string s;
    std::vector<BValue> list;
    std::map<std::string, BValue> dict;

    const BValue* get(const std::string& key) const;
    int64_t getInt(const std::string& key, int64_t def = 0) const;
    std::string getStr(const std::string& key) const;

    static BValue integer(int64_t v);
    static BValue string(const std::string& v);
    static BValue dictionary();
};

// Decodes one value starting at `pos` and advances `pos` past it.
bool bdecode(const std::string& data, size_t& pos, BValue& out, int depth = 0);
std::string bencode(const BValue& v);
