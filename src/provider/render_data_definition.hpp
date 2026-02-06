// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_RENDER_DATA_DEFINE_H
#define RENDERPLUGIN_RENDER_DATA_DEFINE_H

#include <cctype>
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

    enum class LineStyle {
        Solid,
        Dashed
    };

    constexpr auto LINE_STYLE_SOLID = "solid";
    constexpr auto LINE_STYLE_DASHED = "dashed";

    inline std::string lineStyleToString(LineStyle style) {
        return style == LineStyle::Dashed ? LINE_STYLE_DASHED : LINE_STYLE_SOLID;
    }

    inline LineStyle stringToLineStyle(const std::string &str) {
        if (str.empty()) return LineStyle::Solid;
        std::string lower;
        lower.reserve(str.size());
        for (unsigned char c : str) {
            lower.push_back(static_cast<char>(std::tolower(c)));
        }
        if (lower == LINE_STYLE_DASHED) {
            return LineStyle::Dashed;
        }
        return LineStyle::Solid;
    }

    // 文字控制点：左上、上中、右上、左中、中、右中、左下、下中、右下
    enum class TextAnchor {
        TopLeft,
        TopCenter,
        TopRight,
        MidLeft,
        Center,
        MidRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    };

    constexpr auto TEXT_ANCHOR_TOP_LEFT = "topLeft";
    constexpr auto TEXT_ANCHOR_TOP_CENTER = "topCenter";
    constexpr auto TEXT_ANCHOR_TOP_RIGHT = "topRight";
    constexpr auto TEXT_ANCHOR_MID_LEFT = "midLeft";
    constexpr auto TEXT_ANCHOR_CENTER = "center";
    constexpr auto TEXT_ANCHOR_MID_RIGHT = "midRight";
    constexpr auto TEXT_ANCHOR_BOTTOM_LEFT = "bottomLeft";
    constexpr auto TEXT_ANCHOR_BOTTOM_CENTER = "bottomCenter";
    constexpr auto TEXT_ANCHOR_BOTTOM_RIGHT = "bottomRight";

    inline std::string textAnchorToString(TextAnchor anchor) {
        switch (anchor) {
            case TextAnchor::TopLeft: return TEXT_ANCHOR_TOP_LEFT;
            case TextAnchor::TopCenter: return TEXT_ANCHOR_TOP_CENTER;
            case TextAnchor::TopRight: return TEXT_ANCHOR_TOP_RIGHT;
            case TextAnchor::MidLeft: return TEXT_ANCHOR_MID_LEFT;
            case TextAnchor::Center: return TEXT_ANCHOR_CENTER;
            case TextAnchor::MidRight: return TEXT_ANCHOR_MID_RIGHT;
            case TextAnchor::BottomLeft: return TEXT_ANCHOR_BOTTOM_LEFT;
            case TextAnchor::BottomCenter: return TEXT_ANCHOR_BOTTOM_CENTER;
            case TextAnchor::BottomRight: return TEXT_ANCHOR_BOTTOM_RIGHT;
            default: return TEXT_ANCHOR_TOP_LEFT;
        }
    }

    inline TextAnchor stringToTextAnchor(const std::string &str) {
        if (str.empty()) return TextAnchor::TopLeft;
        std::string lower;
        lower.reserve(str.size());
        for (unsigned char c : str) {
            lower.push_back(static_cast<char>(std::tolower(c)));
        }
        if (lower == TEXT_ANCHOR_TOP_CENTER) return TextAnchor::TopCenter;
        if (lower == TEXT_ANCHOR_TOP_RIGHT) return TextAnchor::TopRight;
        if (lower == TEXT_ANCHOR_MID_LEFT) return TextAnchor::MidLeft;
        if (lower == TEXT_ANCHOR_CENTER) return TextAnchor::Center;
        if (lower == TEXT_ANCHOR_MID_RIGHT) return TextAnchor::MidRight;
        if (lower == TEXT_ANCHOR_BOTTOM_LEFT) return TextAnchor::BottomLeft;
        if (lower == TEXT_ANCHOR_BOTTOM_CENTER) return TextAnchor::BottomCenter;
        if (lower == TEXT_ANCHOR_BOTTOM_RIGHT) return TextAnchor::BottomRight;
        return TextAnchor::TopLeft;
    }

    struct RenderData {
        RenderType mType{RenderType::AREA};
        Coordinates mCoordinates{};
        std::string mRawFill{}; // color which will be filled in the area, only used in area type
        Color mFill{};
        std::string mRawColor{}; // line color or text color
        Color mColor{};
        std::wstring mText{}; // text content, supports multi-line with \n
        int mFontSize{}; // text font size
        TextAnchor mTextAnchor{TextAnchor::TopLeft}; // 控制点: topLeft|topCenter|topRight|midLeft|center|midRight|bottomLeft|bottomCenter|bottomRight
        std::string mRawTextBackground{}; // text background color (name or #RRGGBB), empty = no background
        Color mTextBackground{}; // resolved text background color
        std::string mRawTextBackgroundStroke{}; // text background box border color, empty = no stroke
        Color mTextBackgroundStroke{}; // resolved text background stroke color
        float mTextBackgroundStrokeWidth{2.0f}; // text background box border width (px), default 2
        int mZoom{}; // zoom level 1-19, 当前 zoom 小于此值时不绘制；0 表示任意等级都绘制
        LineStyle mLineStyle{LineStyle::Solid}; // line style for LINE type (solid / dashed)
        float mStrokeWidth{0.0f};   // line/outline width, 0 = use default (1.0 solid, 2.0 dashed)
        float mDashLength{0.0f};   // dashed: dash segment length, 0 = use default (10.0)
        float mGapLength{0.0f};     // dashed: gap segment length, 0 = use default (6.0)

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
                                                    mTextAnchor(instance.mTextAnchor),
                                                    mRawTextBackground(std::move(instance.mRawTextBackground)),
                                                    mTextBackground(instance.mTextBackground),
                                                    mRawTextBackgroundStroke(std::move(instance.mRawTextBackgroundStroke)),
                                                    mTextBackgroundStroke(instance.mTextBackgroundStroke),
                                                    mTextBackgroundStrokeWidth(instance.mTextBackgroundStrokeWidth),
                                                    mZoom(instance.mZoom),
                                                    mLineStyle(instance.mLineStyle),
                                                    mStrokeWidth(instance.mStrokeWidth),
                                                    mDashLength(instance.mDashLength),
                                                    mGapLength(instance.mGapLength) {};
    };

    using ColorMap = std::map<std::string, Color>;
    using RenderDataVector = std::vector<RenderData>;
    constexpr auto COLOR_KEY = "color";
    constexpr auto FEATURE_KEY = "features";
}

#endif
