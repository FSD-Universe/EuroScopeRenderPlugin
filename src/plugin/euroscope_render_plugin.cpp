// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include <memory>

#include "euroscope_render_plugin.h"
#include "EuroScopePlugIn.h"
#include "direct2d_render.h"
#include "gdi_plus_render.h"
#include "render_data_yaml_provider.h"

namespace RenderPlugin {
    EuroScopeRenderPlugin::EuroScopeRenderPlugin(HMODULE hModule) :
            EuroScopePlugIn::CPlugIn(EuroScopePlugIn::COMPATIBILITY_CODE,
                                     PLUGIN_NAME,
                                     PLUGIN_VERSION,
                                     PLUGIN_DEVELOPER,
                                     PLUGIN_COPYRIGHT) {
        TCHAR pBuffer[MAX_PATH] = {0};
        GetModuleFileName(hModule, pBuffer, sizeof(pBuffer) / sizeof(TCHAR) - 1);
        mDllPath = pBuffer;

        mConfig = std::make_shared<PluginConfig>();
        readConfig();

        mLogger = std::make_shared<Logger>(mConfig->mLogLevel, mConfig->mLogPath);
        mLogger->debugf("Logger initialized, log level: {}", Logger::getLogLevelName(mConfig->mLogLevel));
        mLogger->debug("Plugin initializing...");
        mLogger->debugf("Data file path: {}", mConfig->mDataFilePath.string());
        mDataProvider = std::make_shared<RenderDataYamlProvider>();
        mDataProvider->loadData(mConfig->mDataFilePath);
        mLogger->debug("Data provider initialized and data loaded");
        mLogger->debugf("Render type: {}", PluginConfig::getRenderTypeName(mConfig->mRenderType));
        if (mConfig->mRenderType == PluginConfig::RenderType::D2D) {
            mRender = std::make_shared<Direct2DRender>();
        } else {
            mRender = std::make_shared<GDIPlusRender>();
        }
        mLogger->debug("Render initialized");
        mRadarScreen = std::make_unique<RadarRender>(mLogger, mDataProvider, mRender);
        mLogger->debug("Radar screen initialized");
        mLogger->debug("Plugin initialized");
    }

    EuroScopeRenderPlugin::~EuroScopeRenderPlugin() {
        if (mRadarScreen != nullptr) {
            mRadarScreen.reset();
        }
        if (mRender != nullptr) {
            mRender.reset();
        }
        if (mDataProvider != nullptr) {
            mDataProvider.reset();
        }
        if (mLogger != nullptr) {
            mLogger.reset();
        }
        if (mConfig != nullptr) {
            mConfig.reset();
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

    bool EuroScopeRenderPlugin::OnCompileCommand(const char *sCommandLine) {
        std::string command(sCommandLine);
        mLogger->debugf("OnCompileCommand: command = {}", command);
        if (command == ".reload") {
            mDataProvider->resetData();
            bool success = mDataProvider->loadData(mConfig->mDataFilePath);
            if (success) {
                displayMessage(DisplayMessage::newMessage("Data file reloaded successfully"));
            } else {
                displayMessage(DisplayMessage::newErrorMessage("Failed to reload data file"));
            }
            return true;
        }
        if (command == ".zoom") {
            std::string message = fmt::format("Current zoom level: {}", mRadarScreen->getCurrentZoomLevel());
            displayMessage(DisplayMessage::newDebugMessage(message));
            return true;
        }
        return false;
    }

    void EuroScopeRenderPlugin::readConfig() {
        std::string logPath = getConfigOrDefault(SETTING_LOG_PATH, DEFAULT_LOG_PATH);
        mConfig->mLogPath = mDllPath.parent_path() / logPath;

        std::string logLevel = getConfigOrDefault(SETTING_LOG_LEVEL, DEFAULT_LOG_LEVEL);
        mConfig->mLogLevel = Logger::getLogLevel(logLevel);

        std::string dataFilePath = getConfigOrDefault(SETTING_CONFIG_PATH, DEFAULT_CONFIG_PATH);
        mConfig->mDataFilePath = mDllPath.parent_path() / dataFilePath;

        std::string renderType = getConfigOrDefault(SETTING_RENDER_TYPE, DEFAULT_RENDER_TYPE);
        mConfig->mRenderType = PluginConfig::getRenderType(renderType);
    }

    std::string EuroScopeRenderPlugin::getConfigOrDefault(const std::string &key, const std::string &defaultValue) {
        auto value = GetDataFromSettings(key.c_str());
        if (value != nullptr) {
            return value;
        }
        return defaultValue;
    }

    void EuroScopeRenderPlugin::displayMessage(const DisplayMessage &message) {
        DisplayUserMessage(message.group.c_str(), message.from.c_str(), message.message.c_str(), message.showHandler,
                           message.showUnread, message.showEvenBusy, message.flashing, message.needConfirm);
    }
}
