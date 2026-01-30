// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_COLOR_UTILS_H
#define RENDERPLUGIN_COLOR_UTILS_H

#include <windows.h>
#include <gdiplus.h>
#include <string>

namespace RenderPlugin {
    inline Gdiplus::Color DEFAULT_COLOR = Gdiplus::Color();

    Gdiplus::Color parseColor(const std::string &colorStr);
}

#endif
