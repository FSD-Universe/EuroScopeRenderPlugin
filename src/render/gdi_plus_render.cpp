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
        const float baseSize = data.mFontSize > 0 ? static_cast<float>(data.mFontSize) : 12.0f;
        const float fontSize = effectiveFontSizePixels > 0.0f ? effectiveFontSizePixels : baseSize;

        Graphics graphics(hdc);
        graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
        SolidBrush brush(data.mColor.gdiColor);
        Font font(L"Euroscope", fontSize, FontStyleRegular, UnitPixel);

        // 先用左对齐测量实际宽高，避免居中对齐时 MeasureString 返回整块布局宽
        StringFormat measureFormat(StringFormat::GenericDefault());
        measureFormat.SetAlignment(StringAlignmentNear);
        measureFormat.SetLineAlignment(StringAlignmentNear);
        measureFormat.SetFormatFlags(measureFormat.GetFormatFlags() | StringFormatFlagsNoWrap);
        const RectF measureRect(0.0f, 0.0f, 4096.0f, 4096.0f);
        RectF boundingBox;
        graphics.MeasureString(data.mText.c_str(), -1, &font, measureRect, &measureFormat, &boundingBox);

        StringAlignment hAlign = StringAlignmentNear;
        switch (data.mTextAnchor) {
            case TextAnchor::Center:
                hAlign = StringAlignmentCenter;
                break;
            case TextAnchor::TopRight:
                hAlign = StringAlignmentFar;
                break;
            default:
                break;
        }
        StringFormat format(StringFormat::GenericDefault());
        format.SetAlignment(hAlign);
        format.SetLineAlignment(StringAlignmentNear);
        format.SetFormatFlags(format.GetFormatFlags() | StringFormatFlagsNoWrap); // 多行仅由 \n 换行

        float left = static_cast<float>(pt.x);
        switch (data.mTextAnchor) {
            case TextAnchor::Center:
                left = static_cast<float>(pt.x) - boundingBox.Width * 0.5f;
                break;
            case TextAnchor::TopRight:
                left = static_cast<float>(pt.x) - boundingBox.Width;
                break;
            default:
                break;
        }
        const RectF drawRect(left, static_cast<float>(pt.y), boundingBox.Width, boundingBox.Height);
        graphics.DrawString(data.mText.c_str(), -1, &font, drawRect, &format, &brush);
    }
}
