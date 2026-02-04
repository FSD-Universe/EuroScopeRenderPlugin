// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include "logger.h"

#include <chrono>
#include <locale>
#include <sstream>
#include <stdexcept>

namespace RenderPlugin {
    Logger::Logger(const LogLevel level, const fs::path &logPath) {
        mLogLevel = static_cast<int>(level);
        if (level == LogLevel::OFF) {
            return;
        }
        if (!fs::exists(logPath.parent_path())) {
            fs::create_directories(logPath.parent_path());
        }
        mLogStream = std::make_unique<std::ofstream>(logPath, std::ios::out | std::ios::app);
        if (!mLogStream->is_open()) {
            throw std::runtime_error("Failed to open log file: " + logPath.string());
        }
        mLogStream->imbue(std::locale());
    }

    Logger::~Logger() {
        if (mLogStream && mLogStream->is_open()) {
            mLogStream->close();
            mLogStream.reset();
        }
    }

    void Logger::log(LogLevel level, const std::string &message) {
        if (!mLogStream || !mLogStream->is_open()) {
            return;
        }
        if (static_cast<int>(level) < mLogLevel) {
            return;
        }
        (*mLogStream) << fmt::format("{} [{}] {}\n",
                                     std::chrono::system_clock::now(),
                                     getLogLevelName(level),
                                     message);
        mLogStream->flush();
    }

    void Logger::debug(const std::string &message) {
        log(LogLevel::DBG, message);
    }

    void Logger::info(const std::string &message) {
        log(LogLevel::INFO, message);
    }

    void Logger::warn(const std::string &message) {
        log(LogLevel::WARN, message);
    }

    void Logger::error(const std::string &message) {
        log(LogLevel::ERR, message);
    }

    void Logger::fatal(const std::string &message) {
        log(LogLevel::FATAL, message);
    }
}
