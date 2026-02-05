// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <dxgiformat.h>
#include <objbase.h>

#include "direct2d_render.h"

namespace RenderPlugin {
    namespace {
        inline D2D1_RECT_F toRectF(const RECT &r) {
            return D2D1::RectF(
                static_cast<float>(r.left),
                static_cast<float>(r.top),
                static_cast<float>(r.right),
                static_cast<float>(r.bottom)
            );
        }
    }

    Direct2DRender::Direct2DRender() {
        // 插件在 EuroScope 主线程加载，宿主通常已初始化 COM(STA)。此处不再调用 CoInitializeEx，
        // 避免与主线程公寓模式冲突导致卡顿或未响应；D2D/DWrite 使用进程内已有 COM 状态。
        mComInitialized = false;
    }

    Direct2DRender::~Direct2DRender() {
        mDCRenderTarget.Reset();
        mDWriteFactory.Reset();
        mD2DFactory.Reset();

        if (mComInitialized) {
            CoUninitialize();
        }
    }

    HRESULT Direct2DRender::ensureDeviceResources() {
        if (!mD2DFactory) {
            HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, mD2DFactory.GetAddressOf());
            if (FAILED(hr)) return hr;
        }

        if (!mDWriteFactory) {
            HRESULT hr = DWriteCreateFactory(
                DWRITE_FACTORY_TYPE_SHARED,
                __uuidof(IDWriteFactory),
                reinterpret_cast<IUnknown **>(mDWriteFactory.GetAddressOf())
            );
            if (FAILED(hr)) return hr;
        }

        if (!mDCRenderTarget) {
            const D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_DEFAULT,
                D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
                0.0f,
                0.0f,
                D2D1_RENDER_TARGET_USAGE_NONE,
                D2D1_FEATURE_LEVEL_DEFAULT
            );

            HRESULT hr = mD2DFactory->CreateDCRenderTarget(&props, mDCRenderTarget.GetAddressOf());
            if (FAILED(hr)) return hr;
        }

        return S_OK;
    }

    HRESULT Direct2DRender::begin(HDC hdc) {
        HRESULT hr = ensureDeviceResources();
        if (FAILED(hr)) return hr;

        RECT rc{};
        if (GetClipBox(hdc, &rc) == ERROR) {
            rc.left = 0;
            rc.top = 0;
            rc.right = 4096;
            rc.bottom = 4096;
        }

        hr = mDCRenderTarget->BindDC(hdc, &rc);
        if (FAILED(hr)) return hr;

        mDCRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        mDCRenderTarget->BeginDraw();
        return S_OK;
    }

    void Direct2DRender::end() {
        if (!mDCRenderTarget) return;
        const HRESULT hr = mDCRenderTarget->EndDraw();
        if (hr == D2DERR_RECREATE_TARGET) {
            mDCRenderTarget.Reset();
        }
    }

    bool Direct2DRender::beginFrame(HDC hdc) {
        return SUCCEEDED(begin(hdc));
    }

    void Direct2DRender::endFrame() {
        end();
    }

    void Direct2DRender::drawLine(HDC hdc, const std::vector<POINT> &points, const RenderData &data) {
        if (points.size() < 2) return;

        const FLOAT strokeWidth = data.mStrokeWidth > 0.0f
            ? data.mStrokeWidth
            : (data.mLineStyle == LineStyle::Dashed ? 2.5f : 1.0f);

        Microsoft::WRL::ComPtr<ID2D1StrokeStyle> dashedStyle;
        ID2D1StrokeStyle *strokeStyle = nullptr;
        if (data.mLineStyle == LineStyle::Dashed && mD2DFactory) {
            const FLOAT dashLen = data.mDashLength > 0.0f ? data.mDashLength : 10.0f;
            const FLOAT gapLen = data.mGapLength > 0.0f ? data.mGapLength : 6.0f;
            const D2D1_STROKE_STYLE_PROPERTIES dashProps = D2D1::StrokeStyleProperties(
                D2D1_CAP_STYLE_FLAT,
                D2D1_CAP_STYLE_FLAT,
                D2D1_CAP_STYLE_FLAT,
                D2D1_LINE_JOIN_MITER,
                10.0f,
                D2D1_DASH_STYLE_CUSTOM,
                0.0f
            );
            const FLOAT dashArray[] = {dashLen, gapLen};
            if (SUCCEEDED(mD2DFactory->CreateStrokeStyle(dashProps, dashArray, 2, dashedStyle.GetAddressOf()))) {
                strokeStyle = dashedStyle.Get();
            }
        }

        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
        if (SUCCEEDED(mDCRenderTarget->CreateSolidColorBrush(data.mColor.d2dColor, brush.GetAddressOf()))) {
            for (size_t i = 1; i < points.size(); ++i) {
                const auto &p0 = points[i - 1];
                const auto &p1 = points[i];
                mDCRenderTarget->DrawLine(
                    D2D1::Point2F(static_cast<float>(p0.x), static_cast<float>(p0.y)),
                    D2D1::Point2F(static_cast<float>(p1.x), static_cast<float>(p1.y)),
                    brush.Get(),
                    strokeWidth,
                    strokeStyle
                );
            }
        }
    }

    void Direct2DRender::drawArea(HDC hdc, const std::vector<POINT> &points, const RenderData &data) {
        if (points.size() < 3) return;

        Microsoft::WRL::ComPtr<ID2D1PathGeometry> geometry;
        if (FAILED(mD2DFactory->CreatePathGeometry(geometry.GetAddressOf()))) {
            return;
        }

        Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
        if (FAILED(geometry->Open(sink.GetAddressOf()))) {
            return;
        }

        sink->SetFillMode(D2D1_FILL_MODE_WINDING);
        sink->BeginFigure(
            D2D1::Point2F(static_cast<float>(points[0].x), static_cast<float>(points[0].y)),
            D2D1_FIGURE_BEGIN_FILLED
        );

        std::vector<D2D1_POINT_2F> linePts;
        linePts.reserve(points.size() - 1);
        for (size_t i = 1; i < points.size(); ++i) {
            linePts.push_back(D2D1::Point2F(static_cast<float>(points[i].x), static_cast<float>(points[i].y)));
        }
        if (!linePts.empty()) {
            sink->AddLines(linePts.data(), static_cast<UINT32>(linePts.size()));
        }
        sink->EndFigure(D2D1_FIGURE_END_CLOSED);
        sink->Close();

        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> fillBrush;
        if (SUCCEEDED(mDCRenderTarget->CreateSolidColorBrush(data.mFill.d2dColor, fillBrush.GetAddressOf()))) {
            mDCRenderTarget->FillGeometry(geometry.Get(), fillBrush.Get());
        }

        const bool drawOutline = !data.mRawColor.empty() || data.mLineStyle == LineStyle::Dashed;
        if (drawOutline) {
            const FLOAT outlineWidth = data.mStrokeWidth > 0.0f
                ? data.mStrokeWidth
                : (data.mLineStyle == LineStyle::Dashed ? 2.5f : 1.0f);

            Microsoft::WRL::ComPtr<ID2D1StrokeStyle> outlineDashedStyle;
            ID2D1StrokeStyle *outlineStrokeStyle = nullptr;
            if (data.mLineStyle == LineStyle::Dashed && mD2DFactory) {
                const FLOAT dashLen = data.mDashLength > 0.0f ? data.mDashLength : 10.0f;
                const FLOAT gapLen = data.mGapLength > 0.0f ? data.mGapLength : 6.0f;
                const D2D1_STROKE_STYLE_PROPERTIES dashProps = D2D1::StrokeStyleProperties(
                    D2D1_CAP_STYLE_FLAT,
                    D2D1_CAP_STYLE_FLAT,
                    D2D1_CAP_STYLE_FLAT,
                    D2D1_LINE_JOIN_MITER,
                    10.0f,
                    D2D1_DASH_STYLE_CUSTOM,
                    0.0f
                );
                const FLOAT dashArray[] = {dashLen, gapLen};
                if (SUCCEEDED(mD2DFactory->CreateStrokeStyle(dashProps, dashArray, 2, outlineDashedStyle.GetAddressOf()))) {
                    outlineStrokeStyle = outlineDashedStyle.Get();
                }
            }
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> strokeBrush;
            if (SUCCEEDED(mDCRenderTarget->CreateSolidColorBrush(data.mColor.d2dColor, strokeBrush.GetAddressOf()))) {
                mDCRenderTarget->DrawGeometry(geometry.Get(), strokeBrush.Get(), outlineWidth, outlineStrokeStyle);
            }
        }
    }

    void Direct2DRender::drawText(HDC hdc, const POINT &pt, const RenderData &data,
                                  float effectiveFontSizePixels) {
        if (data.mText.empty()) return;

        const FLOAT baseSize = data.mFontSize > 0 ? static_cast<FLOAT>(data.mFontSize) : 12.0f;
        const FLOAT fontSize = effectiveFontSizePixels > 0.0f ? effectiveFontSizePixels : baseSize;

        // 先用左对齐布局测量实际宽高，避免 CENTER/TRAILING 时 GetMetrics 返回整块布局宽
        Microsoft::WRL::ComPtr<IDWriteTextFormat> measureFormat;
        if (FAILED(mDWriteFactory->CreateTextFormat(
            L"Euroscope",
            nullptr,
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            fontSize,
            L"",
            measureFormat.GetAddressOf()
        ))) {
            return;
        }
        measureFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        measureFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        measureFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

        constexpr FLOAT maxLayoutSize = 4096.0f;
        Microsoft::WRL::ComPtr<IDWriteTextLayout> measureLayout;
        if (FAILED(mDWriteFactory->CreateTextLayout(
            data.mText.c_str(),
            static_cast<UINT32>(data.mText.length()),
            measureFormat.Get(),
            maxLayoutSize,
            maxLayoutSize,
            measureLayout.GetAddressOf()
        ))) {
            return;
        }
        DWRITE_TEXT_METRICS measureMetrics{};
        if (FAILED(measureLayout->GetMetrics(&measureMetrics))) {
            return;
        }
        const FLOAT contentWidth = measureMetrics.width;
        const FLOAT contentHeight = measureMetrics.height;

        DWRITE_TEXT_ALIGNMENT alignment = DWRITE_TEXT_ALIGNMENT_LEADING;
        switch (data.mTextAnchor) {
            case TextAnchor::Center:
                alignment = DWRITE_TEXT_ALIGNMENT_CENTER;
                break;
            case TextAnchor::TopRight:
                alignment = DWRITE_TEXT_ALIGNMENT_TRAILING;
                break;
            default:
                break;
        }

        Microsoft::WRL::ComPtr<IDWriteTextFormat> format;
        if (FAILED(mDWriteFactory->CreateTextFormat(
            L"Euroscope",
            nullptr,
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            fontSize,
            L"",
            format.GetAddressOf()
        ))) {
            return;
        }
        format->SetTextAlignment(alignment);
        format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

        // 按实际内容尺寸创建绘制用 layout，保证定位正确
        const FLOAT drawLayoutWidth = (contentWidth > 0.0f) ? contentWidth + 1.0f : 1.0f;
        const FLOAT drawLayoutHeight = (contentHeight > 0.0f) ? contentHeight + 1.0f : 1.0f;
        Microsoft::WRL::ComPtr<IDWriteTextLayout> textLayout;
        if (FAILED(mDWriteFactory->CreateTextLayout(
            data.mText.c_str(),
            static_cast<UINT32>(data.mText.length()),
            format.Get(),
            drawLayoutWidth,
            drawLayoutHeight,
            textLayout.GetAddressOf()
        ))) {
            return;
        }

        FLOAT originX = static_cast<FLOAT>(pt.x);
        switch (data.mTextAnchor) {
            case TextAnchor::Center:
                originX = static_cast<FLOAT>(pt.x) - contentWidth * 0.5f;
                break;
            case TextAnchor::TopRight:
                originX = static_cast<FLOAT>(pt.x) - contentWidth;
                break;
            default:
                break;
        }
        const D2D1_POINT_2F origin = D2D1::Point2F(originX, static_cast<FLOAT>(pt.y));

        // 可选：先绘制文字背景矩形（带少量内边距）及背景框描边
        constexpr FLOAT textBackgroundPadding = 2.0f;
        const D2D1_RECT_F bgRect = D2D1::RectF(
            originX - textBackgroundPadding,
            static_cast<FLOAT>(pt.y),
            originX + contentWidth + textBackgroundPadding,
            static_cast<FLOAT>(pt.y) + contentHeight + textBackgroundPadding
        );
        if (!data.mRawTextBackground.empty()) {
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> bgBrush;
            if (SUCCEEDED(mDCRenderTarget->CreateSolidColorBrush(data.mTextBackground.d2dColor, bgBrush.GetAddressOf()))) {
                mDCRenderTarget->FillRectangle(bgRect, bgBrush.Get());
            }
        }
        if (!data.mRawTextBackgroundStroke.empty()) {
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> strokeBrush;
            const FLOAT strokeW = data.mTextBackgroundStrokeWidth > 0.0f ? data.mTextBackgroundStrokeWidth : 2.0f;
            if (SUCCEEDED(mDCRenderTarget->CreateSolidColorBrush(data.mTextBackgroundStroke.d2dColor, strokeBrush.GetAddressOf()))) {
                mDCRenderTarget->DrawRectangle(bgRect, strokeBrush.Get(), strokeW);
            }
        }

        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
        if (SUCCEEDED(mDCRenderTarget->CreateSolidColorBrush(data.mColor.d2dColor, brush.GetAddressOf()))) {
            mDCRenderTarget->DrawTextLayout(origin, textLayout.Get(), brush.Get(),
                D2D1_DRAW_TEXT_OPTIONS_NO_SNAP);
        }
    }
}
