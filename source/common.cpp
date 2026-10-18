#include <algorithm>
#include <cmath>
#include <cstdio>

#include "screens.hpp"

std::string cleanText(const std::string& s) {
    std::string out;
    for (size_t i = 0; i < s.size();) {
        unsigned char c = s[i];
        size_t len = c < 0x80 ? 1 : (c >> 5) == 0x6 ? 2 : (c >> 4) == 0xE ? 3 : (c >> 3) == 0x1E ? 4 : 1;
        if (i + len > s.size()) break;
        bool drop = len == 4;
        if (len == 3) {
            uint32_t cp = ((c & 0x0F) << 12) | ((s[i + 1] & 0x3F) << 6) | (s[i + 2] & 0x3F);
            if ((cp >= 0x2190 && cp <= 0x2BFF) || cp == 0xFE0F || cp == 0x200D) drop = true;
        }
        if (!drop) out.append(s, i, len);
        i += len;
    }
    // collapse the double spaces left behind
    std::string tidy;
    for (char ch : out)
        if (!(ch == ' ' && !tidy.empty() && tidy.back() == ' ')) tidy += ch;
    return tidy;
}

std::vector<std::string> splitLines(const std::string& s) {
    std::vector<std::string> out;
    size_t start = 0;
    while (start <= s.size()) {
        size_t nl = s.find('\n', start);
        std::string line = s.substr(start, nl == std::string::npos ? std::string::npos : nl - start);
        while (!line.empty() && line.front() == ' ') line.erase(0, 1);
        if (!line.empty()) out.push_back(line);
        if (nl == std::string::npos) break;
        start = nl + 1;
    }
    return out;
}

std::string joinFrom(const std::vector<std::string>& v, size_t from, const char* sep) {
    std::string out;
    for (size_t i = from; i < v.size(); i++) {
        if (i > from) out += sep;
        out += v[i];
    }
    return out;
}

std::string joinGenres(const std::vector<std::string>& g, size_t max) {
    std::vector<std::string> v(g.begin(), g.begin() + std::min(max, g.size()));
    return joinFrom(v, 0, "  ·  ");
}

std::string formatClock(double s) {
    if (s < 0 || std::isnan(s)) s = 0;
    int t = (int)s;
    char buf[32];
    if (t >= 3600)
        snprintf(buf, sizeof(buf), "%d:%02d:%02d", t / 3600, (t / 60) % 60, t % 60);
    else
        snprintf(buf, sizeof(buf), "%d:%02d", t / 60, t % 60);
    return buf;
}

std::string formatLeft(double s) {
    int m = (int)(s / 60 + 0.5);
    char buf[32];
    if (m >= 60)
        snprintf(buf, sizeof(buf), "%dh %dm left", m / 60, m % 60);
    else
        snprintf(buf, sizeof(buf), "%dm left", std::max(1, m));
    return buf;
}

std::string formatBytes(double b) {
    char buf[32];
    if (b >= 1024.0 * 1024 * 1024)
        snprintf(buf, sizeof(buf), "%.2f GB", b / (1024.0 * 1024 * 1024));
    else
        snprintf(buf, sizeof(buf), "%.1f MB", b / (1024.0 * 1024));
    return buf;
}

void keepVisible(int sel, int& first, int visible) {
    if (sel < first) first = sel;
    if (sel >= first + visible) first = sel - visible + 1;
    if (first < 0) first = 0;
}

float approach(float current, float target, float speed) {
    float v = current + (target - current) * speed;
    return std::fabs(target - v) < 0.5f ? target : v;
}

// ---------------------------------------------------------------- drawing

static void drawIcon(Gfx& g, ButtonIcon icon, int x, int cy, Color c) {
    switch (icon) {
        case IconPlay: g.triangle(x, cy - 10, x, cy + 10, x + 17, cy, c); break;
        case IconInfo:
            g.roundOutline(x - 1, cy - 11, 22, 22, 11, 2, c);
            g.rect(x + 9, cy - 2, 3, 8, c);
            g.rect(x + 9, cy - 7, 3, 3, c);
            break;
        case IconRestart:
            g.roundOutline(x - 1, cy - 10, 20, 20, 10, 3, c);
            g.triangle(x - 4, cy - 12, x + 6, cy - 12, x + 1, cy - 4, c);
            break;
        case IconList:
            for (int i = 0; i < 3; i++) g.rect(x, cy - 8 + i * 7, 18, 3, c);
            break;
        case IconAdd:
            g.rect(x, cy - 1, 18, 3, c);
            g.rect(x + 7, cy - 9, 3, 19, c);
            break;
        case IconCheck:
            g.triangle(x, cy, x + 3, cy - 3, x + 7, cy + 7, c);
            g.triangle(x + 3, cy - 3, x + 7, cy + 2, x + 7, cy + 7, c);
            g.triangle(x + 7, cy + 7, x + 4, cy + 4, x + 18, cy - 9, c);
            g.triangle(x + 4, cy + 4, x + 15, cy - 11, x + 18, cy - 9, c);
            break;
        case IconNone: break;
    }
}

int buttonWidth(Gfx& g, const std::string& label, ButtonIcon icon) {
    return g.textWidth(label, 22) + (icon != IconNone ? 76 : 52);
}

void drawButton(Gfx& g, int x, int y, int w, int h, const std::string& label, bool focused, bool primary,
                ButtonIcon icon) {
    Color bg = primary ? col::white : col::white.alpha(focused ? 70 : 40);
    Color fg = primary ? col::black : col::white;
    if (focused) {
        g.roundRect(x - 4, y - 4, w + 8, h + 8, (h + 8) / 2, col::accentLight.alpha(220));
        if (!primary) bg = col::white.alpha(235), fg = col::black;
    }
    g.roundRect(x, y, w, h, h / 2, bg);
    int tx = x + 26;
    if (icon != IconNone) {
        drawIcon(g, icon, tx, y + h / 2, fg);
        tx += 30;
    }
    g.text(label, tx, y + (h - 26) / 2, 22, fg);
}

void drawBackdrop(Gfx& g, const std::string& url, int y, Uint8 strength) {
    Uint8 fade = 0;
    if (SDL_Texture* t = g.image(url, &fade)) g.drawCover(t, 0, y, W, H, (Uint8)(fade * strength / 255));
    g.gradientH(0, 0, 860, H, col::bg, 245, 0);
    g.gradientV(0, y + 300, W, 280, col::bg, 0, 255);
    if (y + 580 < H) g.rect(0, y + 580, W, H - (y + 580), col::bg);
    g.gradientV(0, 0, W, 140, col::black, 160, 0);
}

int drawRating(Gfx& g, int x, int y, const std::string& rating) {
    if (rating.empty()) return 0;
    int w1 = g.textWidth("IMDb", 16) + 12;
    g.roundRect(x, y + 2, w1, 22, 4, col::gold);
    g.text("IMDb", x + 6, y + 3, 16, col::black);
    int w2 = g.text(rating, x + w1 + 8, y, 20, col::text);
    return w1 + 8 + w2;
}

void drawProgress(Gfx& g, int x, int y, int w, int h, double p) {
    p = std::max(0.0, std::min(1.0, p));
    g.rect(x, y, w, h, col::white.alpha(60));
    g.rect(x, y, (int)(w * p), h, col::accent);
}

// ---------------------------------------------------------------- contexts

PlayContext contextForMovie(const MetaItem& item) {
    PlayContext c;
    c.metaId = item.id;
    c.type = item.type;
    c.name = item.name;
    c.poster = item.poster;
    c.background = item.background;
    c.logo = item.logo;
    c.videoId = item.id;
    c.genres = item.genres;
    return c;
}

static std::string episodeLabel(const Video& v) {
    std::string l;
    if (v.season > 0) l = "S" + std::to_string(v.season) + ":E" + std::to_string(v.episode);
    if (!v.title.empty()) l += (l.empty() ? "" : "  ·  ") + v.title;
    return l;
}

PlayContext contextForEpisode(const Meta& meta, const Video& v) {
    PlayContext c;
    c.metaId = meta.id;
    c.type = meta.type;
    c.name = meta.name;
    c.poster = meta.poster;
    c.background = meta.background;
    c.logo = meta.logo;
    c.videoId = v.id;
    c.genres = meta.genreList;
    c.episodeLabel = episodeLabel(v);
    // Next in watch order (seasons ascending, specials skipped)
    const Video* next = nullptr;
    for (auto& o : meta.videos) {
        if (o.season <= 0 && v.season > 0) continue;
        bool after = o.season > v.season || (o.season == v.season && o.episode > v.episode);
        if (!after) continue;
        if (!next || o.season < next->season || (o.season == next->season && o.episode < next->episode)) next = &o;
    }
    if (next) {
        c.nextVideoId = next->id;
        c.nextEpisodeLabel = episodeLabel(*next);
    }
    return c;
}

MetaItem itemFromEntry(const HistoryEntry& e) {
    MetaItem m;
    m.id = e.metaId;
    m.type = e.type;
    m.name = e.name;
    m.poster = e.poster;
    m.background = e.background;
    m.logo = e.logo;
    m.genres = e.genres;
    return m;
}
