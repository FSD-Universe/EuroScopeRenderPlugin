// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_STRING_UTILS_H
#define RENDERPLUGIN_STRING_UTILS_H

#include <windows.h>
#include <string>

namespace RenderPlugin {
    std::wstring Utf8ToWstring(const std::string &str);
    std::string WstringToUtf8(const std::wstring &wstr);
}
#endif
