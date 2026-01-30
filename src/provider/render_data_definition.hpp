// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_RENDER_DATA_DEFINE_H
#define RENDERPLUGIN_RENDER_DATA_DEFINE_H

#include <string>
#include <vector>
#include <map>
#include <windows.h>
#include <gdiplus.h>
#include "EuroScopePlugIn.h"

namespace RenderPlugin {
    struct Coordinate {
        double mLongitude{};
        double mLatitude{};

        Coordinate() = default;

        Coordinate(double longitude, double latitude) : mLongitude(longitude), mLatitude(latitude) {}

        [[nodiscard]] EuroScopePlugIn::CPosition toPosition() const {
            EuroScopePlugIn::CPosition pos;
            pos.m_Latitude = mLatitude;
            pos.m_Longitude = mLongitude;
            return pos;
        }
    };

    using Coordinates = std::vector<Coordinate>;
    using ColorMap = std::map<std::string, Gdiplus::Color>;

    enum class RenderType {
        LINE,
        AREA,
        TEXT
    };

    constexpr const char *REMDER_TYPE_TEXT = "text";
    constexpr const char *REMDER_TYPE_LINE = "line";
    constexpr const char *REMDER_TYPE_AREA = "area";

    inline std::string renderTypeToString(RenderType type) {
        switch (type) {
            case RenderType::TEXT:
                return REMDER_TYPE_TEXT;
            case RenderType::LINE:
                return REMDER_TYPE_LINE;
            case RenderType::AREA:
                return REMDER_TYPE_AREA;
            default:
                return REMDER_TYPE_AREA;
        }
    }

    inline RenderType stringToRenderType(const std::string &str) {
        if (str == REMDER_TYPE_TEXT) {
            return RenderType::TEXT;
        }
        if (str == REMDER_TYPE_LINE) {
            return RenderType::LINE;
        }
        return RenderType::AREA;
    }

    struct RenderData {
        RenderType mType{RenderType::AREA};
        Coordinates mCoordinates{};
        std::string mRawFill{}; // color which will be filled in the area, only used in area type
        Gdiplus::Color mFill{};
        std::string mRawColor{}; // line color or text color
        Gdiplus::Color mColor{};
        std::wstring mText{}; // text content
        int mFontSize{}; // text font size

        RenderData() = default;

        RenderData(const RenderData &instance) = default;

        RenderData(RenderData &&instance) noexcept: mType(instance.mType),
                                                    mCoordinates(std::move(instance.mCoordinates)),
                                                    mRawFill(std::move(instance.mRawFill)),
                                                    mFill(instance.mFill),
                                                    mRawColor(std::move(instance.mRawColor)),
                                                    mColor(instance.mColor),
                                                    mText(std::move(instance.mText)),
                                                    mFontSize(instance.mFontSize) {};
    };

    using RenderDataVector = std::vector<RenderData>;
    constexpr const char *COLOR_KEY = "color";
    constexpr const char *FEATURE_KEY = "features";
}

#endif
