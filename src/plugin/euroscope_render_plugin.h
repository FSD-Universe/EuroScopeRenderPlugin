// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_EUROSCOPE_RENDER_PLUGIN_H
#define RENDERPLUGIN_EUROSCOPE_RENDER_PLUGIN_H

#include "EuroScopePlugIn.h"
#include "radar_render.h"

namespace RenderPlugin {
    class EuroScopeRenderPlugin : public EuroScopePlugIn::CPlugIn {
    public:
        EuroScopeRenderPlugin(std::unique_ptr<RadarRender> render);

        ~EuroScopeRenderPlugin() override;

        virtual EuroScopePlugIn::CRadarScreen *
        OnRadarScreenCreated(const char *sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved,
                             bool CanBeCreated) override;

    private:
        std::unique_ptr<RadarRender> mRadarScreen;
    };
}

#endif
