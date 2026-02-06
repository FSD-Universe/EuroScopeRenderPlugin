// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <limits>
#include <sstream>
#include <utility>

#include "radar_render.h"
#include "render_data_definition.hpp"

namespace {

    bool pointInRect(const POINT &p, const RECT &r) {
        return p.x >= r.left && p.x <= r.right && p.y >= r.top && p.y <= r.bottom;
    }

    bool segmentsIntersect(const POINT &a, const POINT &b, const POINT &c, const POINT &d) {
        auto orient = [](const POINT &p, const POINT &q, const POINT &r) {
            return (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
        };
        long o1 = orient(a, b, c), o2 = orient(a, b, d), o3 = orient(c, d, a), o4 = orient(c, d, b);
        if (o1 * o2 > 0 || o3 * o4 > 0) {
            return false;
        }
        if (o1 == 0 && o2 == 0) {
            return (std::min)(a.x, b.x) <= (std::max)(c.x, d.x) && (std::max)(a.x, b.x) >= (std::min)(c.x, d.x) &&
                   (std::min)(a.y, b.y) <= (std::max)(c.y, d.y) && (std::max)(a.y, b.y) >= (std::min)(c.y, d.y);
        }
        return true;
    }

    bool segmentIntersectsRect(const POINT &a, const POINT &b, const RECT &r) {
        if (pointInRect(a, r) || pointInRect(b, r)) {
            return true;
        }
        POINT tl = {r.left, r.top};
        POINT tr = {r.right, r.top};
        POINT br = {r.right, r.bottom};
        POINT bl = {r.left, r.bottom};
        return segmentsIntersect(a, b, tl, tr) || segmentsIntersect(a, b, tr, br) ||
               segmentsIntersect(a, b, br, bl) || segmentsIntersect(a, b, bl, tl);
    }

    bool pointInPolygon(const POINT &p, const std::vector<POINT> &pts) {
        if (pts.size() < 3) {
            return false;
        }
        int n = static_cast<int>(pts.size());
        int crossings = 0;
        for (int i = 0, j = n - 1; i < n; j = i++) {
            const auto &vi = pts[i];
            const auto &vj = pts[j];
            if ((vi.y > p.y) == (vj.y > p.y)) {
                continue;
            }
            double tx = (p.y - vj.y) * (vi.x - vj.x) / (vi.y - vj.y) + vj.x;
            if (static_cast<double>(p.x) < tx) {
                ++crossings;
            }
        }
        return (crossings % 2) == 1;
    }
} // namespace

namespace RenderPlugin {
    RadarRender::RadarRender(std::shared_ptr<Logger> logger, ProviderPtr dataProvider, RenderPtr render,
                             OnClosedCallback onClosed, int textSizeReferenceZoom)
            : mDataProvider(std::move(dataProvider)), mRender(std::move(render)), mLogger(std::move(logger)),
              mOnClosedCallback(std::move(onClosed)), mTextSizeReferenceZoom((std::clamp)(textSizeReferenceZoom, 1, 19)) {}

    RadarRender::~RadarRender() = default;

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

        RECT clipRect{};
        if (GetClipBox(hDC, &clipRect) == ERROR) {
            clipRect = {0, 0, 4096, 4096};
        }

        if (!mRender->beginFrame(hDC)) {
            return;
        }
        for (const auto &data: *renderData) {
            // 当当前缩放等级小于要素配置的 mZoom 时，不绘制该要素
            if (currentZoom < data.mZoom) {
                continue;
            }
            // 线段/文字：至少有一个点在屏幕内才渲染；区域：多边形与屏幕相交即渲染（顶点可在屏幕外）
            bool visible = (data.mType == RenderType::AREA)
                                  ? isAreaIntersectingClip(data, clipRect)
                                  : isAnyPointInClip(data, clipRect);
            if (!visible) {
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
        mRender->endFrame();
    }

    int RadarRender::getCurrentZoomLevel() {
        EuroScopePlugIn::CPosition leftDown{};
        EuroScopePlugIn::CPosition rightUp{};
        GetDisplayArea(&leftDown, &rightUp);

        const double spanLon = rightUp.m_Longitude - leftDown.m_Longitude;
        const double spanLat = rightUp.m_Latitude - leftDown.m_Latitude;
        const double spanDeg = (std::max)(std::abs(spanLon), std::abs(spanLat));

        if (spanDeg <= std::numeric_limits<double>::epsilon()) {
            return 19;
        }

        // 标准地图缩放 1–19：zoom ≈ log2(360 / spanDeg)
        const double rawZoom = std::log2(360.0 / spanDeg);
        int zoom = static_cast<int>(std::round(rawZoom));
        return (std::clamp)(zoom, 1, 19);
    }

    bool RadarRender::isAnyPointInClip(const RenderData &data, const RECT &clipRect) {
        if (data.mCoordinates.empty()) {
            return false;
        }
        for (const auto &coord: data.mCoordinates) {
            POINT pt = ConvertCoordFromPositionToPixel(coord.toPosition());
            if (pt.x >= clipRect.left && pt.x <= clipRect.right &&
                pt.y >= clipRect.top && pt.y <= clipRect.bottom) {
                return true;
            }
        }
        return false;
    }

    bool RadarRender::isAreaIntersectingClip(const RenderData &data, const RECT &clipRect) {
        if (data.mCoordinates.size() < 3) {
            return false;
        }
        std::vector<POINT> points;
        points.reserve(data.mCoordinates.size());
        for (const auto &coord : data.mCoordinates) {
            POINT pt = ConvertCoordFromPositionToPixel(coord.toPosition());
            points.push_back(pt);
        }
        for (const auto &pt : points) {
            if (pointInRect(pt, clipRect)) {
                return true;
            }
        }
        for (size_t i = 0, j = points.size() - 1; i < points.size(); j = i++) {
            if (segmentIntersectsRect(points[j], points[i], clipRect)) {
                return true;
            }
        }
        POINT center = {(clipRect.left + clipRect.right) / 2, (clipRect.top + clipRect.bottom) / 2};
        return pointInPolygon(center, points);
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

    void RadarRender::drawText(HDC hDC, const RenderData &data, double /* spanDeg */) {
        if (data.mCoordinates.empty() || data.mText.empty()) {
            return;
        }

        const auto &coord = data.mCoordinates[0];
        POINT pt = ConvertCoordFromPositionToPixel(coord.toPosition());

        // 文字大小固定为配置的 size（像素），不随视野缩放
        const float effectiveFontSize = data.mFontSize > 0 ? static_cast<float>(data.mFontSize) : 12.0f;
        mRender->drawText(hDC, pt, data, effectiveFontSize);
    }

    void RadarRender::OnAsrContentToBeClosed() {
        if (mOnClosedCallback) {
            mOnClosedCallback(this);
        }
    }
}
