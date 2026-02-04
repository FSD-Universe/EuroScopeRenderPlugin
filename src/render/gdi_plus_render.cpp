// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include <algorithm>
#include "gdi_plus_render.h"

using namespace Gdiplus;

namespace RenderPlugin {
    GDIPlusRender::GDIPlusRender() {
        GdiplusStartup(&mGdiplusToken, &mGdiplusStartupInput, nullptr);
    }

    GDIPlusRender::~GDIPlusRender() {
        GdiplusShutdown(mGdiplusToken);
    }

    void GDIPlusRender::drawLine(HDC hdc, const std::vector<POINT> &points, const RenderData &data) {
        std::vector<Point> point;
        std::for_each(points.begin(), points.end(), [&point](const POINT &p) {
            point.emplace_back(p.x, p.y);
        });

        Graphics graphics(hdc);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        Pen pen(data.mColor.gdiColor, 1.0f);

        graphics.DrawLines(&pen, point.data(), static_cast<int>(point.size()));
    }

    void GDIPlusRender::drawArea(HDC hdc, const std::vector<POINT> &points, const RenderData &data) {
        std::vector<Point> point;
        std::for_each(points.begin(), points.end(), [&point](const POINT &p) {
            point.emplace_back(p.x, p.y);
        });

        Graphics graphics(hdc);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        Pen pen(data.mColor.gdiColor, 1.0f);
        SolidBrush brush(data.mFill.gdiColor);

        graphics.FillPolygon(&brush, point.data(), static_cast<int>(point.size()));
        if (!data.mRawColor.empty()) {
            graphics.DrawPolygon(&pen, point.data(), static_cast<int>(point.size()));
        }
    }

    void GDIPlusRender::drawText(HDC hdc, const POINT &pt, const RenderData &data) {
        PointF point = PointF(pt.x, pt.y);

        Graphics graphics(hdc);
        graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
        SolidBrush brush(data.mColor.gdiColor);
        Font font(L"Euroscope", data.mFontSize, FontStyleRegular, UnitPixel);
        graphics.DrawString(data.mText.c_str(), -1, &font, point, &brush);
    }
}
