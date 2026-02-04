// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include "logger.h"

namespace RenderPlugin {
    Logger::Logger(const fs::path &logPath) : mLogPath(logPath) {
        if (!fs::exists(mLogPath.parent_path())) {
            fs::create_directories(mLogPath.parent_path());
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
}
