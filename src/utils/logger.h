// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_LOGGER_H
#define RENDERPLUGIN_LOGGER_H

#include <filesystem>
#include <fstream>


namespace RenderPlugin {
    namespace fs = std::filesystem;

    enum class LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };

    inline std::string_view getLogLevelName(LogLevel level) {
        constexpr static const std::string_view LOG_LEVEL_NAME[] = {"DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
        return LOG_LEVEL_NAME[int(level)];
    }

    class Logger {
    public:
        Logger(const fs::path &logPath);

        ~Logger();

        void log(LogLevel level, const std::string &message);

        void debug(const std::string &message);

        void debugf(const std::string &message, ...);

        void info(const std::string &message);

        void infof(const std::string &message, ...);

        void warn(const std::string &message);

        void warnf(const std::string &message, ...);

        void error(const std::string &message);

        void errorf(const std::string &message, ...);

        void fatal(const std::string &message);

        void fatalf(const std::string &message, ...);

    private:
        std::unique_ptr<std::ofstream> mLogStream;
        fs::path mLogPath;
    };
}

#endif
