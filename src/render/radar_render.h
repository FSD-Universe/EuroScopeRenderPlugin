// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_RADAR_RENDER_H
#define RENDERPLUGIN_RADAR_RENDER_H

#include <functional>
#include <memory>
#include <windows.h>

#include "logger.h"
#include "render.h"
#include "render_data_provider.h"

namespace RenderPlugin {
    class RadarRender : public EuroScopePlugIn::CRadarScreen {
    public:
        using OnClosedCallback = std::function<void(RadarRender *)>;

        RadarRender(std::shared_ptr<Logger> logger, ProviderPtr dataProvider, RenderPtr render,
                    OnClosedCallback onClosed = nullptr);

        virtual ~RadarRender();

        void OnAsrContentToBeClosed() override;

        void OnRefresh(HDC hDC, int Phase) override;

        int getCurrentZoomLevel();
        /** 当前视野的经纬度跨度（度），用于连续缩放文字等 */
        double getCurrentSpanDeg();

        /** 判断要素是否至少有一个坐标点在裁剪区内（屏幕内） */
        bool isAnyPointInClip(const RenderData &data, const RECT &clipRect);

        void setOnClosedCallback(OnClosedCallback callback) { mOnClosedCallback = std::move(callback); }

    private:
        ProviderPtr mDataProvider;
        RenderPtr mRender;
        std::shared_ptr<Logger> mLogger;
        OnClosedCallback mOnClosedCallback;

        void drawLine(HDC hDC, const RenderData &data);

        void drawArea(HDC hDC, const RenderData &data);

        void drawText(HDC hDC, const RenderData &data, double spanDeg);
    };
}

#endif
