// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_EUROSCOPE_RENDER_DEFINITION_H
#define RENDERPLUGIN_EUROSCOPE_RENDER_DEFINITION_H

#include <filesystem>
#include <string>
#include "logger.h"

namespace RenderPlugin {
    constexpr auto PLUGIN_NAME = "EuroScope Render Plugin";
    constexpr auto PLUGIN_VERSION = "1.0.0";
    constexpr auto PLUGIN_DEVELOPER = "Half_nothing";
    constexpr auto PLUGIN_COPYRIGHT = "MIT License (C) 2026";

    constexpr auto DEFAULT_MESSAGE_GROUP = "RenderPlugin";
    constexpr auto DEFAULT_MESSAGE_SENDER = "System";
    constexpr auto ERROR_MESSAGE_SENDER = "Error";
    constexpr auto DEBUG_MESSAGE_SENDER = "Debug";

    constexpr auto DEFAULT_LOG_PATH = "RenderPlugin.log";
    constexpr auto DEFAULT_CONFIG_PATH = "config.yaml";
    constexpr auto DEFAULT_RENDER_TYPE = "d2d";
    constexpr auto DEFAULT_LOG_LEVEL = "off";

    constexpr auto SETTING_CONFIG_PATH = "ConfigPath";
    constexpr auto SETTING_LOG_PATH = "LogPath";
    constexpr auto SETTING_LOG_LEVEL = "LogLevel";
    // gdi or d2d
    constexpr auto SETTING_RENDER_TYPE = "RenderType";

    namespace fs = std::filesystem;

    struct PluginConfig {
        enum class RenderType {
            D2D,
            GDI
        };

        inline static std::string_view getRenderTypeName(const RenderType &type) {
            constexpr static const std::string_view RENDER_TYPE_NAME[] = {"D2D", "GDI"};
            return RENDER_TYPE_NAME[int(type)];
        }

        inline static RenderType getRenderType(const std::string &type) {
            std::string lowerType = type;
            std::transform(lowerType.begin(), lowerType.end(), lowerType.begin(),
                           [](char c) { return std::toupper(c); });
            if (lowerType == "D2D") {
                return RenderType::D2D;
            } else if (lowerType == "GDI") {
                return RenderType::GDI;
            } else {
                return RenderType::D2D;
            }
        }

        fs::path mDataFilePath;
        fs::path mLogPath;
        Logger::LogLevel mLogLevel;
        RenderType mRenderType;

        PluginConfig() {
            mDataFilePath = fs::current_path() / DEFAULT_CONFIG_PATH;
            mLogPath = fs::current_path() / DEFAULT_LOG_PATH;
            mLogLevel = Logger::LogLevel::DBG;
            mRenderType = RenderType::D2D;
        }
    };
}


#endif
