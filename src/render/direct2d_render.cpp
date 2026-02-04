// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include "direct2d_render.h"

#include <algorithm>
#include <dxgiformat.h>
#include <objbase.h>

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
        // 作为插件被宿主加载时，宿主可能已经初始化了 COM，或使用了不同的 Apartment。
        // 这里尽量“温和”地初始化：如果模式冲突则继续使用现有 COM 环境。
        const HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (SUCCEEDED(hr)) {
            mComInitialized = true;
        } else if (hr == RPC_E_CHANGED_MODE) {
            mComInitialized = false;
        }
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

    void Direct2DRender::drawLine(HDC hdc, const std::vector<POINT> &points, const RenderData &data) {
        if (points.size() < 2) return;
        if (FAILED(begin(hdc))) return;

        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
        if (SUCCEEDED(mDCRenderTarget->CreateSolidColorBrush(data.mColor.d2dColor, brush.GetAddressOf()))) {
            for (size_t i = 1; i < points.size(); ++i) {
                const auto &p0 = points[i - 1];
                const auto &p1 = points[i];
                mDCRenderTarget->DrawLine(
                    D2D1::Point2F(static_cast<float>(p0.x), static_cast<float>(p0.y)),
                    D2D1::Point2F(static_cast<float>(p1.x), static_cast<float>(p1.y)),
                    brush.Get(),
                    1.0f
                );
            }
        }

        end();
    }

    void Direct2DRender::drawArea(HDC hdc, const std::vector<POINT> &points, const RenderData &data) {
        if (points.size() < 3) return;
        if (FAILED(begin(hdc))) return;

        Microsoft::WRL::ComPtr<ID2D1PathGeometry> geometry;
        if (FAILED(mD2DFactory->CreatePathGeometry(geometry.GetAddressOf()))) {
            end();
            return;
        }

        Microsoft::WRL::ComPtr<ID2D1GeometrySink> sink;
        if (FAILED(geometry->Open(sink.GetAddressOf()))) {
            end();
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

        if (!data.mRawColor.empty()) {
            Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> strokeBrush;
            if (SUCCEEDED(mDCRenderTarget->CreateSolidColorBrush(data.mColor.d2dColor, strokeBrush.GetAddressOf()))) {
                mDCRenderTarget->DrawGeometry(geometry.Get(), strokeBrush.Get(), 1.0f);
            }
        }

        end();
    }

    void Direct2DRender::drawText(HDC hdc, const POINT &pt, const RenderData &data) {
        if (data.mText.empty()) return;
        if (FAILED(begin(hdc))) return;

        Microsoft::WRL::ComPtr<IDWriteTextFormat> format;
        const FLOAT fontSize = data.mFontSize > 0 ? static_cast<FLOAT>(data.mFontSize) : 12.0f;
        if (SUCCEEDED(mDWriteFactory->CreateTextFormat(
            L"Euroscope",
            nullptr,
            DWRITE_FONT_WEIGHT_REGULAR,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            fontSize,
            L"",
            format.GetAddressOf()
        ))) {
            format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
            format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
        }

        RECT rc{};
        if (GetClipBox(hdc, &rc) == ERROR) {
            rc.left = 0;
            rc.top = 0;
            rc.right = 4096;
            rc.bottom = 4096;
        }

        D2D1_RECT_F layout = toRectF(rc);
        layout.left = static_cast<float>(pt.x);
        layout.top = static_cast<float>(pt.y);

        Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush;
        if (SUCCEEDED(mDCRenderTarget->CreateSolidColorBrush(data.mColor.d2dColor, brush.GetAddressOf()))) {
            mDCRenderTarget->DrawTextA(
                data.mText.c_str(),
                static_cast<UINT32>(data.mText.size()),
                format.Get(),
                layout,
                brush.Get(),
                D2D1_DRAW_TEXT_OPTIONS_NO_SNAP,
                DWRITE_MEASURING_MODE_NATURAL
            );
        }

        end();
    }
}
