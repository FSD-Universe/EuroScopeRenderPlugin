// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include <cmath>
#include <filesystem>
#include <limits>
#include <sstream>

#include "radar_render.h"

namespace RenderPlugin {
    RadarRender::RadarRender(std::shared_ptr<Logger> logger, ProviderPtr dataProvider, RenderPtr render)
            : mDataProvider(dataProvider), mRender(render), mLogger(logger) {}

    RadarRender::~RadarRender() {}

    void RadarRender::OnRefresh(HDC hDC, int Phase) {
        if (!mDataProvider->isLoaded() || Phase != EuroScopePlugIn::REFRESH_PHASE_BACK_BITMAP) {
            return;
        }

        auto renderData = mDataProvider->getRenderData();
        if (!renderData) {
            return;
        }

        const int currentZoom = getCurrentZoomLevel();
        const double currentSpanDeg = getCurrentSpanDeg();

        for (const auto &data: *renderData) {
            // 当当前缩放等级小于要素配置的 mZoom 时，不绘制该要素
            if (currentZoom < data.mZoom) {
                continue;
            }
            switch (data.mType) {
                case RenderType::LINE:
                    drawLine(hDC, data);
                    break;
                case RenderType::AREA:
                    drawArea(hDC, data);
                    break;
                case RenderType::TEXT:
                    drawText(hDC, data, currentSpanDeg);
                    break;
            }
        }
    }

    int RadarRender::getCurrentZoomLevel() {
        EuroScopePlugIn::CPosition leftDown{};
        EuroScopePlugIn::CPosition rightUp{};
        GetDisplayArea(&leftDown, &rightUp);

        const double spanLon = rightUp.m_Longitude - leftDown.m_Longitude;
        const double spanLat = rightUp.m_Latitude - leftDown.m_Latitude;
        const double spanDeg = (std::max)(std::abs(spanLon), std::abs(spanLat));

        if (spanDeg <= std::numeric_limits<double>::epsilon()) {
            // 极端情况下认为是最大缩放（19 级）
            return 19;
        }

        // 将当前显示的经纬度跨度映射为标准地图缩放级别 1–19。
        // 近似采用 Web 地图的定义：zoom ≈ log2(360 / spanDeg)，然后裁剪到 [1, 19]。
        const double rawZoom = std::log2(360.0 / spanDeg);
        int zoom = static_cast<int>(std::round(rawZoom));
        zoom = std::clamp(zoom, 1, 19);
        return zoom;
    }

    double RadarRender::getCurrentSpanDeg() {
        EuroScopePlugIn::CPosition leftDown{};
        EuroScopePlugIn::CPosition rightUp{};
        GetDisplayArea(&leftDown, &rightUp);

        const double spanLon = rightUp.m_Longitude - leftDown.m_Longitude;
        const double spanLat = rightUp.m_Latitude - leftDown.m_Latitude;
        double spanDeg = (std::max)(std::abs(spanLon), std::abs(spanLat));
        if (spanDeg <= std::numeric_limits<double>::epsilon()) {
            spanDeg = 360.0 / 1024.0;
        }
        return spanDeg;
    }

    void RadarRender::drawLine(HDC hDC, const RenderData &data) {
        if (data.mCoordinates.size() < 2) {
            return;
        }

        std::vector<POINT> points;
        for (const auto &coord: data.mCoordinates) {
            POINT pt = ConvertCoordFromPositionToPixel(coord.toPosition());
            points.push_back(pt);
        }

        mRender->drawLine(hDC, points, data);
    }

    void RadarRender::drawArea(HDC hDC, const RenderData &data) {
        if (data.mCoordinates.size() < 3) {
            return;
        }

        std::vector<POINT> points;
        for (const auto &coord: data.mCoordinates) {
            POINT pt = ConvertCoordFromPositionToPixel(coord.toPosition());
            points.push_back(pt);
        }

        mRender->drawArea(hDC, points, data);
    }

    void RadarRender::drawText(HDC hDC, const RenderData &data, double spanDeg) {
        if (data.mCoordinates.empty() || data.mText.empty()) {
            return;
        }

        const auto &coord = data.mCoordinates[0];
        POINT pt = ConvertCoordFromPositionToPixel(coord.toPosition());

        // 用连续的比例（视野跨度）缩放字体，避免按整数 zoom 时字体跳变
        // 参考：spanDeg = 360/1024 时缩放系数为 1（对应约 zoom 10）
        constexpr double referenceSpanDeg = 360.0 / 1024.0;
        const float baseSize = data.mFontSize > 0 ? static_cast<float>(data.mFontSize) : 12.0f;
        const float scaleFactor = static_cast<float>(referenceSpanDeg / spanDeg);
        const float effectiveFontSize = baseSize * scaleFactor;

        mRender->drawText(hDC, pt, data, effectiveFontSize);
    }

    void RadarRender::OnAsrContentToBeClosed() {}
}
