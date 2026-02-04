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
        const float penWidth = data.mStrokeWidth > 0.0f
            ? data.mStrokeWidth
            : (data.mLineStyle == LineStyle::Dashed ? 2.0f : 1.0f);
        Pen pen(data.mColor.gdiColor, penWidth);
        if (data.mLineStyle == LineStyle::Dashed) {
            if (data.mDashLength > 0.0f && data.mGapLength > 0.0f) {
                const float w = (std::max)(penWidth, 0.1f);
                REAL dashPattern[] = {data.mDashLength / w, data.mGapLength / w};
                pen.SetDashPattern(dashPattern, 2);
            } else {
                pen.SetDashStyle(DashStyleDash);
            }
        }

        graphics.DrawLines(&pen, point.data(), static_cast<int>(point.size()));
    }

    void GDIPlusRender::drawArea(HDC hdc, const std::vector<POINT> &points, const RenderData &data) {
        std::vector<Point> point;
        std::for_each(points.begin(), points.end(), [&point](const POINT &p) {
            point.emplace_back(p.x, p.y);
        });

        Graphics graphics(hdc);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        SolidBrush brush(data.mFill.gdiColor);
        graphics.FillPolygon(&brush, point.data(), static_cast<int>(point.size()));

        const bool drawOutline = !data.mRawColor.empty() || data.mLineStyle == LineStyle::Dashed;
        if (drawOutline) {
            const float outlineWidth = data.mStrokeWidth > 0.0f
                ? data.mStrokeWidth
                : (data.mLineStyle == LineStyle::Dashed ? 2.0f : 1.0f);
            Pen outlinePen(data.mColor.gdiColor, outlineWidth);
            if (data.mLineStyle == LineStyle::Dashed) {
                if (data.mDashLength > 0.0f && data.mGapLength > 0.0f) {
                    const float w = (std::max)(outlineWidth, 0.1f);
                    REAL dashPattern[] = {data.mDashLength / w, data.mGapLength / w};
                    outlinePen.SetDashPattern(dashPattern, 2);
                } else {
                    outlinePen.SetDashStyle(DashStyleDash);
                }
            }
            graphics.DrawPolygon(&outlinePen, point.data(), static_cast<int>(point.size()));
        }
    }

    void GDIPlusRender::drawText(HDC hdc, const POINT &pt, const RenderData &data,
                                float effectiveFontSizePixels) {
        PointF point = PointF(pt.x, pt.y);

        const float baseSize = data.mFontSize > 0 ? static_cast<float>(data.mFontSize) : 12.0f;
        const float fontSize = effectiveFontSizePixels > 0.0f ? effectiveFontSizePixels : baseSize;

        Graphics graphics(hdc);
        graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
        SolidBrush brush(data.mColor.gdiColor);
        Font font(L"Euroscope", fontSize, FontStyleRegular, UnitPixel);
        graphics.DrawString(data.mText.c_str(), -1, &font, point, &brush);
    }
}
