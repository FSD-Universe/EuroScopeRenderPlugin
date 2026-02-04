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
                    drawText(hDC, data);
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

    void RadarRender::drawText(HDC hDC, const RenderData &data) {
        if (data.mCoordinates.empty() || data.mText.empty()) {
            return;
        }

        const auto &coord = data.mCoordinates[0];
        POINT pt = ConvertCoordFromPositionToPixel(coord.toPosition());

        mRender->drawText(hDC, pt, data);
    }

    void RadarRender::OnAsrContentToBeClosed() {}
}
