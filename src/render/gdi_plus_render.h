// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_GDI_PLUS_RENDER_H
#define RENDERPLUGIN_GDI_PLUS_RENDER_H

#include "render.h"

namespace RenderPlugin {
    class GDIPlusRender : public Render {
    public:
        GDIPlusRender();

        ~GDIPlusRender() override;

        void drawLine(HDC hdc, const std::vector<POINT> &points, const RenderData &data) override;

        void drawArea(HDC hdc, const std::vector<POINT> &points, const RenderData &data) override;

        void drawText(HDC hdc, const POINT &pt, const RenderData &data,
                     float effectiveFontSizePixels = 0.0f) override;

    private:
        Gdiplus::GdiplusStartupInput mGdiplusStartupInput;
        ULONG_PTR mGdiplusToken{};
    };
}


#endif
