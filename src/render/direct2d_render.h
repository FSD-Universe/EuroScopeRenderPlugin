// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_DIRECT2D_RENDER_H
#define RENDERPLUGIN_DIRECT2D_RENDER_H

#include <d2d1.h>
#include <dwrite.h>
#include <wrl/client.h>

#include "render.h"

namespace RenderPlugin {
    class Direct2DRender : public Render {
    public:
        Direct2DRender();

        ~Direct2DRender() override;

        void drawLine(HDC hdc, const std::vector<POINT> &points, const RenderData &data) override;

        void drawArea(HDC hdc, const std::vector<POINT> &points, const RenderData &data) override;

        void drawText(HDC hdc, const POINT &pt, const RenderData &data,
                     float effectiveFontSizePixels = 0.0f) override;

    private:
        bool mComInitialized{false};

        Microsoft::WRL::ComPtr<ID2D1Factory> mD2DFactory;
        Microsoft::WRL::ComPtr<IDWriteFactory> mDWriteFactory;
        Microsoft::WRL::ComPtr<ID2D1DCRenderTarget> mDCRenderTarget;

        HRESULT ensureDeviceResources();
        HRESULT begin(HDC hdc);

        void end();
    };
}

#endif
