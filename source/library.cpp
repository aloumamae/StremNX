#include "library.hpp"

#include <algorithm>

#include "log.hpp"

static const char* kLibraryPath = APP_DIR "/library.json";

void Library::load() {
    std::string text;
    if (!readFile(kLibraryPath, text)) return;
    bool ok = false;
    Json j = Json::parse(text, &ok);
    if (!ok) return;
    list.clear();
    for (auto& e : j["items"].items()) {
        MetaItem m;
        m.id = e["id"].str();
        m.type = e["type"].str();
        m.name = e["name"].str();
        m.poster = e["poster"].str();
        m.background = e["background"].str();
        m.logo = e["logo"].str();
        if (!m.id.empty()) list.push_back(m);
    }
}

void Library::save() const {
    Json arr = Json::array();
    for (auto& m : list) {
        Json e = Json::object();
        e.set("id", m.id).set("type", m.type).set("name", m.name).set("poster", m.poster);
        e.set("background", m.background).set("logo", m.logo);
        arr.push(e);
    }
    Json j = Json::object();
    j.set("items", arr);
    std::string text = j.dump();
    writeFile(kLibraryPath, text.data(), text.size());
}

bool Library::contains(const std::string& id) const {
    return std::any_of(list.begin(), list.end(), [&](const MetaItem& m) { return m.id == id; });
}

void Library::add(const MetaItem& m) {
    remove(m.id);
    list.insert(list.begin(), m);
    save();
}

void Library::remove(const std::string& id) {
    list.erase(std::remove_if(list.begin(), list.end(), [&](const MetaItem& m) { return m.id == id; }), list.end());
    save();
}

void Library::replace(const std::vector<MetaItem>& items) {
    list = items;
    save();
}
