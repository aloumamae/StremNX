#pragma once

#include <switch.h>

#include <cstdint>

enum Key : uint32_t {
    K_UP = 1 << 0,
    K_DOWN = 1 << 1,
    K_LEFT = 1 << 2,
    K_RIGHT = 1 << 3,
    K_A = 1 << 4,
    K_B = 1 << 5,
    K_X = 1 << 6,
    K_Y = 1 << 7,
    K_L = 1 << 8,
    K_R = 1 << 9,
    K_ZL = 1 << 10,
    K_ZR = 1 << 11,
    K_PLUS = 1 << 12,
    K_MINUS = 1 << 13,
};

// Controller input (D-pad and left stick are equivalent), with repeat while held.
// Touch screen: swipes act like the D-pad, a short tap is reported through tap().
class Input {
public:
    void init();
    void update();
    bool pressed(uint32_t k) const { return (pressed_ & k) != 0; }
    bool held(uint32_t k) const { return (held_ & k) != 0; }
    // A tap that ended this frame (screen coordinates, 1280x720).
    bool tap(int& x, int& y) const {
        if (!tapNow) return false;
        x = tapX;
        y = tapY;
        return true;
    }
    bool touching() const { return tDown; }

private:
    PadState pad;
    uint32_t pressed_ = 0, held_ = 0;
    uint32_t repeatStart[4] = {}, repeatLast[4] = {};
    bool tDown = false, tMoved = false, tapNow = false;
    int sx = 0, sy = 0, lx = 0, ly = 0, accX = 0, accY = 0, tapX = 0, tapY = 0;
    uint32_t tStart = 0;
};
