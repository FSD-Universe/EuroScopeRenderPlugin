// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include "euroscope_render_definition.h"
#include "euroscope_render_plugin.h"

namespace RenderPlugin {
    EuroScopeRenderPlugin::EuroScopeRenderPlugin(std::unique_ptr<RadarRender> render) :
            EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE, PLUGIN_NAME, PLUGIN_VERSION,
                                     PLUGIN_DEVELOPER, PLUGIN_COPYRIGHT), mRadarScreen(std::move(render)) {}

    EuroScopeRenderPlugin::~EuroScopeRenderPlugin() {
        if (mRadarScreen != nullptr) {
            mRadarScreen.reset();
        }
    }

    EuroScopePlugIn::CRadarScreen *
    EuroScopeRenderPlugin::OnRadarScreenCreated(const char *sDisplayName,
                                                bool NeedRadarContent,
                                                bool GeoReferenced,
                                                bool CanBeSaved,
                                                bool CanBeCreated) {
        return mRadarScreen.get();
    }
}
