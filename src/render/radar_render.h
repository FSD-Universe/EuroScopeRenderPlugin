// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_RADAR_RENDER_H
#define RENDERPLUGIN_RADAR_RENDER_H

#include <memory>
#include <windows.h>

#include "logger.h"
#include "render.h"
#include "render_data_provider.h"

namespace RenderPlugin {
    class RadarRender : public EuroScopePlugIn::CRadarScreen {
    public:
        RadarRender(std::shared_ptr<Logger> logger, ProviderPtr dataProvider, RenderPtr render);

        virtual ~RadarRender();

        virtual void OnAsrContentToBeClosed() override;

        void OnRefresh(HDC hDC, int Phase) override;

        int getCurrentZoomLevel();
        /** 当前视野的经纬度跨度（度），用于连续缩放文字等 */
        double getCurrentSpanDeg();

    private:
        ProviderPtr mDataProvider;
        RenderPtr mRender;
        std::shared_ptr<Logger> mLogger;

        void drawLine(HDC hDC, const RenderData &data);

        void drawArea(HDC hDC, const RenderData &data);

        void drawText(HDC hDC, const RenderData &data, double spanDeg);
    };
}

#endif
