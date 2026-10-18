#pragma once

#include <string>
#include <vector>

// Minimal JSON: enough for manifests, catalogs and the Stremio API.
class Json {
public:
    enum Type { Null, Bool, Number, String, Array, Object };

    Json() {}
    Json(bool v) : t(Bool), b(v) {}
    Json(int v) : t(Number), n(v) {}
    Json(double v) : t(Number), n(v) {}
    Json(const char* v) : t(String), s(v) {}
    Json(const std::string& v) : t(String), s(v) {}

    static Json array();
    static Json object();
    static Json parse(const std::string& text, bool* ok = nullptr);

    Type type() const { return t; }
    bool isNull() const { return t == Null; }
    bool isBool() const { return t == Bool; }
    bool isNumber() const { return t == Number; }
    bool isString() const { return t == String; }
    bool isArray() const { return t == Array; }
    bool isObject() const { return t == Object; }

    // Lenient access: a missing key/index returns null.
    const Json& operator[](const std::string& key) const;
    const Json& operator[](const char* key) const { return (*this)[std::string(key)]; }
    const Json& operator[](size_t i) const;
    bool has(const std::string& key) const;
    size_t size() const;

    std::string str(const std::string& def = "") const;
    double num(double def = 0) const;
    bool boolean(bool def = false) const;

    const std::vector<Json>& items() const { return vals; }
    const std::vector<std::string>& keys() const { return ks; }

    Json& set(const std::string& key, Json v);
    void push(Json v);

    std::string dump() const;

private:
    void dumpTo(std::string& out) const;

    Type t = Null;
    bool b = false;
    double n = 0;
    std::string s;
    std::vector<std::string> ks;  // chaves (Object)
    std::vector<Json> vals;       // valores (Array e Object)
};
