#include "input.hpp"

#include <SDL2/SDL.h>

#include <cstdlib>

static uint32_t mapButtons(u64 b) {
    uint32_t k = 0;
    if (b & (HidNpadButton_Up | HidNpadButton_StickLUp)) k |= K_UP;
    if (b & (HidNpadButton_Down | HidNpadButton_StickLDown)) k |= K_DOWN;
    if (b & (HidNpadButton_Left | HidNpadButton_StickLLeft)) k |= K_LEFT;
    if (b & (HidNpadButton_Right | HidNpadButton_StickLRight)) k |= K_RIGHT;
    if (b & HidNpadButton_A) k |= K_A;
    if (b & HidNpadButton_B) k |= K_B;
    if (b & HidNpadButton_X) k |= K_X;
    if (b & HidNpadButton_Y) k |= K_Y;
    if (b & HidNpadButton_L) k |= K_L;
    if (b & HidNpadButton_R) k |= K_R;
    if (b & HidNpadButton_ZL) k |= K_ZL;
    if (b & HidNpadButton_ZR) k |= K_ZR;
    if (b & HidNpadButton_Plus) k |= K_PLUS;
    if (b & HidNpadButton_Minus) k |= K_MINUS;
    return k;
}

void Input::init() {
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    padInitializeDefault(&pad);
    hidInitializeTouchScreen();
}

void Input::update() {
    padUpdate(&pad);
    pressed_ = mapButtons(padGetButtonsDown(&pad));
    held_ = mapButtons(padGetButtons(&pad));

    const uint32_t now = SDL_GetTicks();
    const uint32_t dirs[4] = {K_UP, K_DOWN, K_LEFT, K_RIGHT};
    for (int i = 0; i < 4; i++) {
        if (pressed_ & dirs[i]) {
            repeatStart[i] = repeatLast[i] = now;
        } else if ((held_ & dirs[i]) && now - repeatStart[i] > 350 && now - repeatLast[i] > 70) {
            pressed_ |= dirs[i];
            repeatLast[i] = now;
        }
    }

    // Touch
    tapNow = false;
    HidTouchScreenState ts = {};
    if (hidGetTouchScreenStates(&ts, 1) && ts.count > 0) {
        int x = (int)ts.touches[0].x, y = (int)ts.touches[0].y;
        if (!tDown) {
            tDown = true;
            tMoved = false;
            sx = lx = x;
            sy = ly = y;
            accX = accY = 0;
            tStart = now;
        } else {
            accX += x - lx;
            accY += y - ly;
            lx = x;
            ly = y;
            if (std::abs(x - sx) > 24 || std::abs(y - sy) > 24) tMoved = true;
            // Dragging the content: finger to the left shows what is on the right, and so on.
            for (; accX <= -110; accX += 110) pressed_ |= K_RIGHT;
            for (; accX >= 110; accX -= 110) pressed_ |= K_LEFT;
            for (; accY <= -90; accY += 90) pressed_ |= K_DOWN;
            for (; accY >= 90; accY -= 90) pressed_ |= K_UP;
        }
    } else if (tDown) {
        tDown = false;
        if (!tMoved && now - tStart < 500) {
            tapNow = true;
            tapX = lx;
            tapY = ly;
        }
    }
}
