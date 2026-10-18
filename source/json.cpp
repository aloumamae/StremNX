#include "json.hpp"

#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

static const Json kNull;

Json Json::array() {
    Json j;
    j.t = Array;
    return j;
}

Json Json::object() {
    Json j;
    j.t = Object;
    return j;
}

const Json& Json::operator[](const std::string& key) const {
    if (t != Object) return kNull;
    for (size_t i = 0; i < ks.size(); i++)
        if (ks[i] == key) return vals[i];
    return kNull;
}

const Json& Json::operator[](size_t i) const {
    if (t != Array || i >= vals.size()) return kNull;
    return vals[i];
}

bool Json::has(const std::string& key) const {
    if (t != Object) return false;
    for (auto& k : ks)
        if (k == key) return true;
    return false;
}

size_t Json::size() const {
    return (t == Array || t == Object) ? vals.size() : 0;
}

std::string Json::str(const std::string& def) const {
    if (t == String) return s;
    if (t == Number) {
        char buf[64];
        if (std::floor(n) == n && std::fabs(n) < 1e15)
            snprintf(buf, sizeof(buf), "%lld", (long long)n);
        else
            snprintf(buf, sizeof(buf), "%g", n);
        return buf;
    }
    return def;
}

double Json::num(double def) const {
    if (t == Number) return n;
    if (t == String && !s.empty()) {
        char* end = nullptr;
        double v = strtod(s.c_str(), &end);
        if (end != s.c_str()) return v;
    }
    return def;
}

bool Json::boolean(bool def) const {
    return t == Bool ? b : def;
}

Json& Json::set(const std::string& key, Json v) {
    if (t != Object) {
        *this = object();
    }
    for (size_t i = 0; i < ks.size(); i++) {
        if (ks[i] == key) {
            vals[i] = std::move(v);
            return *this;
        }
    }
    ks.push_back(key);
    vals.push_back(std::move(v));
    return *this;
}

void Json::push(Json v) {
    if (t != Array) *this = array();
    vals.push_back(std::move(v));
}

// ---------------------------------------------------------------- serialization

static void dumpString(std::string& out, const std::string& s) {
    out += '"';
    for (unsigned char c : s) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += (char)c;
                }
        }
    }
    out += '"';
}

void Json::dumpTo(std::string& out) const {
    switch (t) {
        case Null: out += "null"; break;
        case Bool: out += b ? "true" : "false"; break;
        case Number: {
            char buf[64];
            if (std::floor(n) == n && std::fabs(n) < 1e15)
                snprintf(buf, sizeof(buf), "%lld", (long long)n);
            else
                snprintf(buf, sizeof(buf), "%.17g", n);
            out += buf;
            break;
        }
        case String: dumpString(out, s); break;
        case Array:
            out += '[';
            for (size_t i = 0; i < vals.size(); i++) {
                if (i) out += ',';
                vals[i].dumpTo(out);
            }
            out += ']';
            break;
        case Object:
            out += '{';
            for (size_t i = 0; i < vals.size(); i++) {
                if (i) out += ',';
                dumpString(out, ks[i]);
                out += ':';
                vals[i].dumpTo(out);
            }
            out += '}';
            break;
    }
}

std::string Json::dump() const {
    std::string out;
    dumpTo(out);
    return out;
}

// ---------------------------------------------------------------- parser

namespace {

struct Parser {
    const char* p;
    const char* end;
    bool ok = true;

    void ws() {
        while (p < end && (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t')) ++p;
    }

    bool match(const char* lit) {
        size_t n = strlen(lit);
        if ((size_t)(end - p) >= n && memcmp(p, lit, n) == 0) {
            p += n;
            return true;
        }
        return false;
    }

    static void utf8(std::string& o, uint32_t cp) {
        if (cp < 0x80) {
            o += (char)cp;
        } else if (cp < 0x800) {
            o += (char)(0xC0 | (cp >> 6));
            o += (char)(0x80 | (cp & 0x3F));
        } else if (cp < 0x10000) {
            o += (char)(0xE0 | (cp >> 12));
            o += (char)(0x80 | ((cp >> 6) & 0x3F));
            o += (char)(0x80 | (cp & 0x3F));
        } else {
            o += (char)(0xF0 | (cp >> 18));
            o += (char)(0x80 | ((cp >> 12) & 0x3F));
            o += (char)(0x80 | ((cp >> 6) & 0x3F));
            o += (char)(0x80 | (cp & 0x3F));
        }
    }

    bool hex4(uint32_t& v) {
        if (end - p < 4) return false;
        v = 0;
        for (int i = 0; i < 4; i++) {
            char c = *p++;
            v <<= 4;
            if (c >= '0' && c <= '9') v |= c - '0';
            else if (c >= 'a' && c <= 'f') v |= c - 'a' + 10;
            else if (c >= 'A' && c <= 'F') v |= c - 'A' + 10;
            else return false;
        }
        return true;
    }

    bool parseString(std::string& o) {
        ++p;  // aspas de abertura
        while (p < end) {
            char c = *p++;
            if (c == '"') return true;
            if (c != '\\') {
                o += c;
                continue;
            }
            if (p >= end) return false;
            char e = *p++;
            switch (e) {
                case '"': o += '"'; break;
                case '\\': o += '\\'; break;
                case '/': o += '/'; break;
                case 'b': o += '\b'; break;
                case 'f': o += '\f'; break;
                case 'n': o += '\n'; break;
                case 'r': o += '\r'; break;
                case 't': o += '\t'; break;
                case 'u': {
                    uint32_t cp;
                    if (!hex4(cp)) return false;
                    if (cp >= 0xD800 && cp <= 0xDBFF) {
                        if (end - p >= 6 && p[0] == '\\' && p[1] == 'u') {
                            p += 2;
                            uint32_t lo;
                            if (!hex4(lo)) return false;
                            if (lo >= 0xDC00 && lo <= 0xDFFF)
                                cp = 0x10000 + ((cp - 0xD800) << 10) + (lo - 0xDC00);
                            else
                                cp = 0xFFFD;
                        } else {
                            cp = 0xFFFD;
                        }
                    }
                    utf8(o, cp);
                    break;
                }
                default: return false;
            }
        }
        return false;
    }

    Json number() {
        const char* start = p;
        if (*p == '-') ++p;
        while (p < end && (isdigit((unsigned char)*p) || *p == '.' || *p == 'e' || *p == 'E' ||
                           *p == '+' || *p == '-'))
            ++p;
        std::string tmp(start, p);
        char* e = nullptr;
        double d = strtod(tmp.c_str(), &e);
        if (e == tmp.c_str()) ok = false;
        return Json(d);
    }

    Json value(int depth) {
        if (depth > 128) {
            ok = false;
            return Json();
        }
        ws();
        if (p >= end) {
            ok = false;
            return Json();
        }
        char c = *p;
        if (c == '{') return object(depth);
        if (c == '[') return array(depth);
        if (c == '"') {
            std::string s;
            if (!parseString(s)) ok = false;
            return Json(s);
        }
        if (c == 't' && match("true")) return Json(true);
        if (c == 'f' && match("false")) return Json(false);
        if (c == 'n' && match("null")) return Json();
        if (c == '-' || (c >= '0' && c <= '9')) return number();
        ok = false;
        return Json();
    }

    Json array(int depth) {
        ++p;
        Json a = Json::array();
        ws();
        if (p < end && *p == ']') {
            ++p;
            return a;
        }
        while (ok) {
            a.push(value(depth + 1));
            ws();
            if (p >= end) { ok = false; break; }
            if (*p == ',') { ++p; continue; }
            if (*p == ']') { ++p; break; }
            ok = false;
        }
        return a;
    }

    Json object(int depth) {
        ++p;
        Json o = Json::object();
        ws();
        if (p < end && *p == '}') {
            ++p;
            return o;
        }
        while (ok) {
            ws();
            if (p >= end || *p != '"') { ok = false; break; }
            std::string k;
            if (!parseString(k)) { ok = false; break; }
            ws();
            if (p >= end || *p != ':') { ok = false; break; }
            ++p;
            o.set(k, value(depth + 1));
            ws();
            if (p >= end) { ok = false; break; }
            if (*p == ',') { ++p; continue; }
            if (*p == '}') { ++p; break; }
            ok = false;
        }
        return o;
    }
};

}  // namespace

Json Json::parse(const std::string& text, bool* okOut) {
    Parser ps{text.data(), text.data() + text.size()};
    Json v = ps.value(0);
    if (okOut) *okOut = ps.ok;
    return ps.ok ? v : Json();
}
