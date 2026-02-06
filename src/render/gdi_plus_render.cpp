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
        StringAlignment vAlign = StringAlignmentNear;
        switch (data.mTextAnchor) {
            case TextAnchor::TopCenter:
            case TextAnchor::Center:
            case TextAnchor::BottomCenter:
                hAlign = StringAlignmentCenter;
                break;
            case TextAnchor::TopRight:
            case TextAnchor::MidRight:
            case TextAnchor::BottomRight:
                hAlign = StringAlignmentFar;
                break;
            default:
                break;
        }
        switch (data.mTextAnchor) {
            case TextAnchor::MidLeft:
            case TextAnchor::Center:
            case TextAnchor::MidRight:
                vAlign = StringAlignmentCenter;
                break;
            case TextAnchor::BottomLeft:
            case TextAnchor::BottomCenter:
            case TextAnchor::BottomRight:
                vAlign = StringAlignmentFar;
                break;
            default:
                break;
        }
        StringFormat format(StringFormat::GenericDefault());
        format.SetAlignment(hAlign);
        format.SetLineAlignment(vAlign);
        format.SetFormatFlags(format.GetFormatFlags() | StringFormatFlagsNoWrap);

        float left = static_cast<float>(pt.x);
        float top = static_cast<float>(pt.y);
        switch (data.mTextAnchor) {
            case TextAnchor::TopCenter:
            case TextAnchor::Center:
            case TextAnchor::BottomCenter:
                left = static_cast<float>(pt.x) - boundingBox.Width * 0.5f;
                break;
            case TextAnchor::TopRight:
            case TextAnchor::MidRight:
            case TextAnchor::BottomRight:
                left = static_cast<float>(pt.x) - boundingBox.Width;
                break;
            default:
                break;
        }
        switch (data.mTextAnchor) {
            case TextAnchor::MidLeft:
            case TextAnchor::Center:
            case TextAnchor::MidRight:
                top = static_cast<float>(pt.y) - boundingBox.Height * 0.5f;
                break;
            case TextAnchor::BottomLeft:
            case TextAnchor::BottomCenter:
            case TextAnchor::BottomRight:
                top = static_cast<float>(pt.y) - boundingBox.Height;
                break;
            default:
                break;
        }
        const RectF drawRect(left, top, boundingBox.Width, boundingBox.Height);

        constexpr float textBackgroundPadding = 2.0f;
        const RectF bgRect(
            left - textBackgroundPadding,
            top - textBackgroundPadding,
            boundingBox.Width + textBackgroundPadding * 2.0f,
            boundingBox.Height + textBackgroundPadding * 2.0f
        );
        if (!data.mRawTextBackground.empty()) {
            SolidBrush bgBrush(data.mTextBackground.gdiColor);
            graphics.FillRectangle(&bgBrush, bgRect);
        }
        if (!data.mRawTextBackgroundStroke.empty()) {
            const float strokeW = data.mTextBackgroundStrokeWidth > 0.0f ? data.mTextBackgroundStrokeWidth : 2.0f;
            Pen strokePen(data.mTextBackgroundStroke.gdiColor, strokeW);
            graphics.DrawRectangle(&strokePen, bgRect);
        }

        graphics.DrawString(data.mText.c_str(), -1, &font, drawRect, &format, &brush);
    }
}
