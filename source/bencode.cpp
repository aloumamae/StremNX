#include "bencode.hpp"

#include <cstdio>
#include <cstdlib>

const BValue* BValue::get(const std::string& key) const {
    if (type != Dict) return nullptr;
    auto it = dict.find(key);
    return it == dict.end() ? nullptr : &it->second;
}

int64_t BValue::getInt(const std::string& key, int64_t def) const {
    const BValue* v = get(key);
    return v && v->type == Int ? v->i : def;
}

std::string BValue::getStr(const std::string& key) const {
    const BValue* v = get(key);
    return v && v->type == Str ? v->s : std::string();
}

BValue BValue::integer(int64_t v) {
    BValue b;
    b.type = Int;
    b.i = v;
    return b;
}

BValue BValue::string(const std::string& v) {
    BValue b;
    b.type = Str;
    b.s = v;
    return b;
}

BValue BValue::dictionary() {
    BValue b;
    b.type = Dict;
    return b;
}

bool bdecode(const std::string& data, size_t& pos, BValue& out, int depth) {
    if (depth > 64 || pos >= data.size()) return false;
    char c = data[pos];
    if (c == 'i') {
        size_t e = data.find('e', pos + 1);
        if (e == std::string::npos) return false;
        out.type = BValue::Int;
        out.i = strtoll(data.c_str() + pos + 1, nullptr, 10);
        pos = e + 1;
        return true;
    }
    if (c == 'l') {
        out.type = BValue::List;
        pos++;
        while (pos < data.size() && data[pos] != 'e') {
            BValue v;
            if (!bdecode(data, pos, v, depth + 1)) return false;
            out.list.push_back(std::move(v));
        }
        if (pos >= data.size()) return false;
        pos++;
        return true;
    }
    if (c == 'd') {
        out.type = BValue::Dict;
        pos++;
        while (pos < data.size() && data[pos] != 'e') {
            BValue k, v;
            if (!bdecode(data, pos, k, depth + 1) || k.type != BValue::Str) return false;
            if (!bdecode(data, pos, v, depth + 1)) return false;
            out.dict[k.s] = std::move(v);
        }
        if (pos >= data.size()) return false;
        pos++;
        return true;
    }
    if (c >= '0' && c <= '9') {
        size_t colon = data.find(':', pos);
        if (colon == std::string::npos) return false;
        long long len = strtoll(data.c_str() + pos, nullptr, 10);
        if (len < 0 || colon + 1 + (size_t)len > data.size()) return false;
        out.type = BValue::Str;
        out.s = data.substr(colon + 1, (size_t)len);
        pos = colon + 1 + (size_t)len;
        return true;
    }
    return false;
}

static void encodeTo(std::string& out, const BValue& v) {
    char buf[32];
    switch (v.type) {
        case BValue::Int:
            snprintf(buf, sizeof(buf), "i%llde", (long long)v.i);
            out += buf;
            break;
        case BValue::Str:
            snprintf(buf, sizeof(buf), "%zu:", v.s.size());
            out += buf;
            out += v.s;
            break;
        case BValue::List:
            out += 'l';
            for (auto& x : v.list) encodeTo(out, x);
            out += 'e';
            break;
        case BValue::Dict:
            out += 'd';
            for (auto& kv : v.dict) {  // std::map keeps keys sorted, as bencode requires
                snprintf(buf, sizeof(buf), "%zu:", kv.first.size());
                out += buf;
                out += kv.first;
                encodeTo(out, kv.second);
            }
            out += 'e';
            break;
        case BValue::None: break;
    }
}

std::string bencode(const BValue& v) {
    std::string out;
    encodeTo(out, v);
    return out;
}
