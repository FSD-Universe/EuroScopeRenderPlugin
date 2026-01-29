// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_COLOR_UTILS_H
#define RENDERPLUGIN_COLOR_UTILS_H

#include <string>
#include <windef.h>
#include <wingdi.h>

namespace RenderPlugin {
    constexpr COLORREF DEFAULT_COLOR = RGB(0, 0, 0);

    COLORREF parseColor(const std::string &colorStr);
}

#endif
