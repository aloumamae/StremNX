#pragma once

#include <switch.h>

#include <string>

// System on-screen keyboard (swkbd). Returns false if the user cancels.
inline bool showKeyboard(std::string& out, const std::string& guide, const std::string& initial = "",
                         bool password = false) {
    SwkbdConfig kbd;
    char buf[512] = {0};
    if (R_FAILED(swkbdCreate(&kbd, 0))) return false;
    if (password)
        swkbdConfigMakePresetPassword(&kbd);
    else
        swkbdConfigMakePresetDefault(&kbd);
    swkbdConfigSetGuideText(&kbd, guide.c_str());
    swkbdConfigSetInitialText(&kbd, initial.c_str());
    swkbdConfigSetStringLenMax(&kbd, sizeof(buf) - 1);
    Result rc = swkbdShow(&kbd, buf, sizeof(buf));
    swkbdClose(&kbd);
    if (R_FAILED(rc)) return false;
    out = buf;
    return true;
}
