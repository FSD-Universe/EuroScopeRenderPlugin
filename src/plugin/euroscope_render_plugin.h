// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_EUROSCOPE_RENDER_PLUGIN_H
#define RENDERPLUGIN_EUROSCOPE_RENDER_PLUGIN_H

#include <vector>
#include "EuroScopePlugIn.h"
#include "euroscope_render_definition.h"
#include "render_data_provider.h"
#include "render.h"
#include "radar_render.h"

namespace RenderPlugin {
    struct DisplayMessage {
        std::string message{};
        std::string group{};
        std::string from{};
        bool showHandler{};
        bool showUnread{};
        bool showEvenBusy{};
        bool flashing{};
        bool needConfirm{};

        DisplayMessage(const std::string &message) :
                DisplayMessage(message, DEFAULT_MESSAGE_SENDER) {}

        DisplayMessage(const std::string &message, const std::string &from) :
                message(message), group(DEFAULT_MESSAGE_GROUP), from(from), showHandler(true), showUnread(true) {}

        static DisplayMessage newDebugMessage(const std::string &message) {
            return DisplayMessage(message, DEBUG_MESSAGE_SENDER);
        }

        static DisplayMessage newMessage(const std::string &message) {
            return DisplayMessage(message);
        }

        static DisplayMessage newErrorMessage(const std::string &message) {
            return DisplayMessage(message, ERROR_MESSAGE_SENDER);
        }
    };

    class EuroScopeRenderPlugin : public EuroScopePlugIn::CPlugIn {
    public:
        EuroScopeRenderPlugin(HMODULE hModule);

        ~EuroScopeRenderPlugin() override;

        virtual EuroScopePlugIn::CRadarScreen *
        OnRadarScreenCreated(const char *sDisplayName, bool NeedRadarContent, bool GeoReferenced, bool CanBeSaved,
                             bool CanBeCreated) override;

        virtual bool OnCompileCommand(const char *sCommandLine) override;

        /** 由 RadarRender 在 OnAsrContentToBeClosed 时调用，用于延迟移除已关闭的屏幕 */
        void notifyRadarScreenClosed(RadarRender *screen);

    private:
        fs::path mDllPath;
        std::shared_ptr<Logger> mLogger;
        std::shared_ptr<PluginConfig> mConfig;
        RenderPtr mRender;
        ProviderPtr mDataProvider;
        std::vector<std::unique_ptr<RadarRender>> mRadarScreens;
        std::vector<RadarRender *> mRadarScreensToRemove;

        void removeClosedRadarScreens();
        void readConfig();

        std::string getConfigOrDefault(const std::string &key, const std::string &defaultValue);

        void displayMessage(const DisplayMessage &message);
    };
}

#endif
