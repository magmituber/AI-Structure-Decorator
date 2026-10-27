#pragma once

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace autodeco {
inline std::vector<std::string> splitString(std::string const& s, char sep) {
    std::vector<std::string> out; std::string cur;
    for (char c : s) { if (c == sep) { out.push_back(cur); cur.clear(); } else cur.push_back(c); }
    out.push_back(cur); return out;
}
inline std::string formatFloat(double v) {
    char buf[64]; std::snprintf(buf, sizeof(buf), "%.4f", v); std::string s(buf);
    if (s.find('.') != std::string::npos) { while (!s.empty() && s.back() == '0') s.pop_back(); if (!s.empty() && s.back() == '.') s.pop_back(); }
    if (s.empty() || s == "-0") s = "0"; return s;
}
inline std::set<int> const& keptKeys() {
    static const std::set<int> keys = {4, 5, 6, 32, 128, 129, 130, 131, 132}; return keys;
}
inline std::string buildOverlayString(std::string raw, int overlayID, float dx = 0.f, float dy = 0.f, int zLayer = 9, int zOrder = 10) {
    auto semi = raw.find(';'); if (semi != std::string::npos) raw = raw.substr(0, semi);
    while (!raw.empty() && (raw.back() == '\n' || raw.back() == '\r' || raw.back() == ',')) raw.pop_back();
    if (raw.empty()) return "";
    auto parts = splitString(raw, ','); std::string x, y;
    std::vector<std::pair<std::string, std::string>> kept;
    for (size_t i = 0; i + 1 < parts.size(); i += 2) {
        auto const& k = parts[i]; auto const& v = parts[i + 1]; int key = std::atoi(k.c_str());
        if (key == 2) x = v; else if (key == 3) y = v; else if (keptKeys().count(key)) kept.emplace_back(k, v);
    }
    if (x.empty() || y.empty()) return "";
    if (dx != 0.f) x = formatFloat(std::atof(x.c_str()) + dx);
    if (dy != 0.f) y = formatFloat(std::atof(y.c_str()) + dy);
    std::string out = "1," + std::to_string(overlayID) + ",2," + x + ",3," + y;
    for (auto const& [k, v] : kept) out += "," + k + "," + v;
    out += ",21,1011,22,1011,24," + std::to_string(zLayer) + ",25," + std::to_string(zOrder) + ";";
    return out;
}
} // namespace autodeco
