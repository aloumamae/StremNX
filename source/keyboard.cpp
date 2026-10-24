#include "keyboard.hpp"

#include <cctype>
#include <cmath>
#include <vector>

#include "app.hpp"
#include "log.hpp"

namespace {

enum KeyAction { KChar, KShift, KSpace, KBack, KDone };

struct KbKey {
    std::string label;
    KeyAction act;
    int row, col, span;  // grid of 10 columns
};

std::vector<KbKey> layout() {
    static const char* rows[] = {"1234567890", "qwertyuiop", "asdfghjkl'", "zxcvbnm,.-", "@_:/?&=#+!"};
    std::vector<KbKey> keys;
    for (int r = 0; r < 5; r++)
        for (int c = 0; c < 10; c++) keys.push_back({std::string(1, rows[r][c]), KChar, r, c, 1});
    keys.push_back({"Shift", KShift, 5, 0, 2});
    keys.push_back({"Space", KSpace, 5, 2, 4});
    keys.push_back({"Delete", KBack, 5, 6, 2});
    keys.push_back({"Done", KDone, 5, 8, 2});
    return keys;
}

constexpr int kX0 = 120, kY0 = 226, kUnit = 104, kGap = 8, kKeyH = 60, kRowStep = 68, kFieldW = 10 * kUnit - kGap;

SDL_Rect keyRect(const KbKey& k) {
    return {kX0 + k.col * kUnit, kY0 + k.row * kRowStep, k.span * kUnit - kGap, kKeyH};
}

std::string upper(std::string s) {
    for (auto& c : s) c = (char)toupper((unsigned char)c);
    return s;
}

void popChar(std::string& s) {  // one UTF-8 character
    while (!s.empty() && ((unsigned char)s.back() & 0xC0) == 0x80) s.pop_back();
    if (!s.empty()) s.pop_back();
}

}  // namespace

bool showKeyboard(App& app, std::string& out, const std::string& guide, const std::string& initial, bool password) {
    Gfx& g = app.gfx;
    Input& in = app.in;
    std::vector<KbKey> keys = layout();
    std::string text = initial;
    bool shift = false;
    int sel = 10;  // "q"

    while (appletMainLoop()) {
        SDL_Event e;
        while (SDL_PollEvent(&e))
            if (e.type == SDL_QUIT) return false;
        watchBeat(WatchMain, "keyboard");
        in.update();
        g.beginFrame();
        g.pumpImages();

        // D-pad: left/right along the row, up/down to the key under the same spot.
        const KbKey& cur = keys[sel];
        int dx = in.pressed(K_RIGHT) ? 1 : in.pressed(K_LEFT) ? -1 : 0;
        if (dx && sel + dx >= 0 && sel + dx < (int)keys.size() && keys[sel + dx].row == cur.row) sel += dx;
        int dy = in.pressed(K_DOWN) ? 1 : in.pressed(K_UP) ? -1 : 0;
        if (dy) {
            float mid = cur.col + cur.span / 2.0f, best = 1e9f;
            for (int i = 0; i < (int)keys.size(); i++) {
                float d = std::fabs(keys[i].col + keys[i].span / 2.0f - mid);
                if (keys[i].row == cur.row + dy && d < best) best = d, sel = i;
            }
        }

        // A (or a touch / pointer click on a key) types the key.
        int press = in.pressed(K_A) ? sel : -1;
        int tx = 0, ty = 0;
        if (in.tap(tx, ty))
            for (int i = 0; i < (int)keys.size(); i++) {
                SDL_Rect r = keyRect(keys[i]);
                if (tx >= r.x && tx < r.x + r.w && ty >= r.y && ty < r.y + r.h) press = sel = i;
            }
        bool done = in.pressed(K_PLUS);
        if (press >= 0) {
            const KbKey& k = keys[press];
            if (k.act == KChar) {
                text += shift ? upper(k.label) : k.label;
                shift = false;
            } else if (k.act == KShift) {
                shift = !shift;
            } else if (k.act == KSpace) {
                text += ' ';
            } else if (k.act == KBack) {
                popChar(text);
            } else {
                done = true;
            }
        }
        if (in.pressed(K_X)) shift = !shift;
        if (in.pressed(K_Y)) text += ' ';
        if (in.pressed(K_B)) {
            if (text.empty()) return false;
            popChar(text);
        }
        if (in.pressed(K_MINUS)) return false;
        if (done) {
            out = text;
            return true;
        }

        g.clear(col::bg);
        g.text(guide, kX0, 60, 26, col::dim, kFieldW);
        g.roundRect(kX0, 110, kFieldW, 72, 14, col::card);
        g.roundOutline(kX0, 110, kFieldW, 72, 14, 2, col::accentLight);
        std::string shown;
        if (password)
            for (char c : text) shown += ((unsigned char)c & 0xC0) == 0x80 ? "" : "*";
        else
            shown = text;
        const int maxW = kFieldW - 60;
        while (!shown.empty() && g.textWidth(shown, 30) > maxW) {  // the end is what is being typed
            shown.erase(0, 1);
            while (!shown.empty() && ((unsigned char)shown[0] & 0xC0) == 0x80) shown.erase(0, 1);
        }
        int tw = shown.empty() ? 0 : g.text(shown, kX0 + 24, 128, 30, col::text);
        if (SDL_GetTicks() / 500 % 2) g.rect(kX0 + 26 + tw, 124, 3, 44, col::accentLight);  // caret

        for (int i = 0; i < (int)keys.size(); i++) {
            const KbKey& k = keys[i];
            SDL_Rect r = keyRect(k);
            Color bg = k.act == KDone                ? col::accent
                       : k.act == KShift && shift    ? col::accentLight.alpha(110)
                       : i == sel                    ? col::cardSel
                                                     : col::card;
            g.roundRect(r.x, r.y, r.w, r.h, 12, bg);
            if (i == sel) g.roundOutline(r.x, r.y, r.w, r.h, 12, 2, col::accentLight);
            std::string label = k.act == KChar && shift ? upper(k.label) : k.label;
            g.text(label, r.x + r.w / 2, r.y + (k.act == KChar ? 12 : 16), k.act == KChar ? 28 : 21, col::text, 0,
                   AlignCenter);
        }
        app.drawFooter("A Type   B Delete   X Shift   Y Space   + Done   - Cancel");
        app.drawPointer();
        g.present();
    }
    return false;
}
