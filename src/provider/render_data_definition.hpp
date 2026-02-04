// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_RENDER_DATA_DEFINE_H
#define RENDERPLUGIN_RENDER_DATA_DEFINE_H

#include <d2d1.h>
#include <gdiplus.h>
#include <map>
#include <string>
#include <vector>
#include <windows.h>

#include "EuroScopePlugIn.h"

namespace RenderPlugin {
    struct Color {
        using uchar = unsigned char;

        uchar red;
        uchar green;
        uchar blue;
        uchar alpha;
        Gdiplus::Color gdiColor{};
        D2D1_COLOR_F d2dColor{};

        Color() noexcept: Color(0, 0, 0) {}

        Color(uchar red, uchar green, uchar blue) noexcept: Color(red, green, blue, 255) {}

        Color(uchar red, uchar green, uchar blue, uchar alpha) noexcept
                : red(red), green(green), blue(blue), alpha(alpha) {
            updateDerivedColors();
        }

        static Color fromColorString(const std::string &color) {
            if (color.empty() || color[0] != '#') {
                return Color(0, 0, 0);
            }

            std::string hex = color.substr(1);
            if (hex.length() == 3) {          // #RGB
                hex = expandShortHex(hex, false);
            } else if (hex.length() == 4) {   // #RGBA
                hex = expandShortHex(hex, true);
            } else if (hex.length() != 6 && hex.length() != 8) {
                return Color(0, 0, 0);
            }

            try {
                uchar r = std::stoi(hex.substr(0, 2), nullptr, 16);
                uchar g = std::stoi(hex.substr(2, 2), nullptr, 16);
                uchar b = std::stoi(hex.substr(4, 2), nullptr, 16);
                if (hex.length() == 6) {
                    return Color(r, g, b);
                }
                uchar a = std::stoi(hex.substr(6, 2), nullptr, 16);
                return Color(r, g, b, a);
            } catch (...) {
                return Color(0, 0, 0);
            }
        }

        void updateDerivedColors() {
            gdiColor = Gdiplus::Color(alpha, red, green, blue);
            d2dColor = D2D1::ColorF(red / 255.0f, green / 255.0f, blue / 255.0f, alpha / 255.0f);
        }

    private:
        static std::string expandShortHex(const std::string &shortHex, bool hasAlpha) {
            std::string expanded;
            expanded.reserve(hasAlpha ? 8 : 6);

            for (char c: shortHex) {
                expanded.push_back(c);
                expanded.push_back(c);
            }

            return expanded;
        }
    };

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
        Color mFill{};
        std::string mRawColor{}; // line color or text color
        Color mColor{};
        std::wstring mText{}; // text content
        int mFontSize{}; // text font size
        int mZoom{}; // zoom level, when current zoom level is less than this value, the render data will be ignored

        RenderData() = default;

        RenderData(const RenderData &instance) = default;

        RenderData(RenderData &&instance) noexcept: mType(instance.mType),
                                                    mCoordinates(std::move(instance.mCoordinates)),
                                                    mRawFill(std::move(instance.mRawFill)),
                                                    mFill(instance.mFill),
                                                    mRawColor(std::move(instance.mRawColor)),
                                                    mColor(instance.mColor),
                                                    mText(std::move(instance.mText)),
                                                    mFontSize(instance.mFontSize),
                                                    mZoom(instance.mZoom) {};
    };

    using ColorMap = std::map<std::string, Color>;
    using RenderDataVector = std::vector<RenderData>;
    constexpr const char *COLOR_KEY = "color";
    constexpr const char *FEATURE_KEY = "features";
}

#endif
