// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include "color_utils.h"

namespace RenderPlugin {
    COLORREF parseColor(const std::string &colorStr) {
        if (colorStr.empty() || colorStr[0] != '#') {
            return DEFAULT_COLOR;
        }

        std::string hex = colorStr.substr(1);
        if (hex.length() == 3) {
            // FFF -> FFFFFF
            hex = hex.substr(0, 1) + hex.substr(0, 1) + hex.substr(1, 1) +
                  hex.substr(1, 1) + hex.substr(2, 1) + hex.substr(2, 1);
        }
        if (hex.length() != 6) {
            return DEFAULT_COLOR;
        }

        try {
            int r = std::stoi(hex.substr(0, 2), nullptr, 16);
            int g = std::stoi(hex.substr(2, 2), nullptr, 16);
            int b = std::stoi(hex.substr(4, 2), nullptr, 16);
            return RGB(r, g, b);
        } catch (...) {
            return DEFAULT_COLOR;
        }
    }
}
