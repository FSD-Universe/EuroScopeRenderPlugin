// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_LOGGER_H
#define RENDERPLUGIN_LOGGER_H

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <string_view>

#include <fmt/core.h>
#include <fmt/chrono.h>

namespace RenderPlugin {
    namespace fs = std::filesystem;

    class Logger {
    public:
        enum class LogLevel {
            OFF,
            DBG,
            INFO,
            WARN,
            ERR,
            FATAL
        };

        inline static std::string_view getLogLevelName(LogLevel level) {
            constexpr static const std::string_view LOG_LEVEL_NAME[] = {"OFF", "DEBUG", "INFO", "WARN", "ERROR",
                                                                        "FATAL"};
            return LOG_LEVEL_NAME[int(level)];
        }

        inline static LogLevel getLogLevel(const std::string &level) {
            std::string levelStr = level;
            std::transform(levelStr.begin(), levelStr.end(), levelStr.begin(), [](char c) { return std::toupper(c); });
            if (levelStr == "DEBUG") {
                return LogLevel::DBG;
            } else if (levelStr == "INFO") {
                return LogLevel::INFO;
            } else if (levelStr == "WARN") {
                return LogLevel::WARN;
            } else if (levelStr == "ERROR") {
                return LogLevel::ERR;
            } else if (levelStr == "FATAL") {
                return LogLevel::FATAL;
            } else {
                return LogLevel::OFF;
            }
        }

        Logger(const LogLevel level, const fs::path &logPath);

        ~Logger();

        void log(LogLevel level, const std::string &message);

        void debug(const std::string &message);

        template<typename... Args>
        void debugf(fmt::format_string<Args...> fmtStr, Args &&...args) {
            debug(fmt::format(fmtStr, std::forward<Args>(args)...));
        }

        void info(const std::string &message);

        template<typename... Args>
        void infof(fmt::format_string<Args...> fmtStr, Args &&...args) {
            info(fmt::format(fmtStr, std::forward<Args>(args)...));
        }

        void warn(const std::string &message);

        template<typename... Args>
        void warnf(fmt::format_string<Args...> fmtStr, Args &&...args) {
            warn(fmt::format(fmtStr, std::forward<Args>(args)...));
        }

        void error(const std::string &message);

        template<typename... Args>
        void errorf(fmt::format_string<Args...> fmtStr, Args &&...args) {
            error(fmt::format(fmtStr, std::forward<Args>(args)...));
        }

        void fatal(const std::string &message);

        template<typename... Args>
        void fatalf(fmt::format_string<Args...> fmtStr, Args &&...args) {
            fatal(fmt::format(fmtStr, std::forward<Args>(args)...));
        }

    private:
        std::unique_ptr<std::ofstream> mLogStream;
        int mLogLevel;
    };
}

#endif
