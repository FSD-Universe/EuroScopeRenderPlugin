// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_RADAR_RENDER_H
#define RENDERPLUGIN_RADAR_RENDER_H

#include "render_data_provider.h"
#include "EuroScopePlugIn.h"
#include "render.h"
#include <windows.h>
#include <memory>

namespace RenderPlugin {
    class RadarRender : public EuroScopePlugIn::CRadarScreen {
    public:
        RadarRender(ProviderPtr dataProvider, RenderPtr render, fs::path configPath);

        virtual ~RadarRender();

        virtual void OnAsrContentToBeClosed() override;

        void OnRefresh(HDC hDC, int Phase) override;

        bool OnCompileCommand(const char *sCommandLine) override;

    private:
        ProviderPtr mDataProvider;
        RenderPtr mRender;
        fs::path mConfigPath;
        bool mIsLoaded;

        void drawLine(HDC hDC, const RenderData &data);

        void drawArea(HDC hDC, const RenderData &data);

        void drawText(HDC hDC, const RenderData &data);


    };
}

#endif
