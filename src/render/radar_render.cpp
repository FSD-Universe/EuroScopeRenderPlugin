// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include <sstream>
#include <filesystem>
#include "radar_render.h"
#include "render_data_provider.h"

namespace RenderPlugin {
    RadarRender::RadarRender(ProviderPtr dataProvider, fs::path configPath) : mDataProvider(std::move(dataProvider)),
                                                                              mConfigPath(std::move(configPath)),
                                                                              mIsLoaded(false) {
        mIsLoaded = mDataProvider->loadData(configPath.string());
    }

    RadarRender::~RadarRender() {
        if (mIsLoaded) {
            mDataProvider->resetData();
            mDataProvider.reset();
            mIsLoaded = false;
        }
    }

    void RadarRender::drawLine(HDC hDC, const RenderData &data) {
        if (data.mCoordinates.size() < 2) {
            return;
        }

        std::vector<POINT> points;
        for (const auto &coord: data.mCoordinates) {
            EuroScopePlugIn::CPosition pos;
            pos.m_Latitude = coord.mLatitude;
            pos.m_Longitude = coord.mLongitude;
            POINT pt = ConvertCoordFromPositionToPixel(pos);
            points.push_back(pt);
        }

        HPEN hPen = CreatePen(PS_SOLID, 1, data.mColor);
        HPEN hOldPen = (HPEN) SelectObject(hDC, hPen);

        Polyline(hDC, points.data(), static_cast<int>(points.size()));

        SelectObject(hDC, hOldPen);
        DeleteObject(hPen);
    }

    void RadarRender::OnRefresh(HDC hDC, int Phase) {
        if (!mIsLoaded || Phase != EuroScopePlugIn::REFRESH_PHASE_AFTER_TAGS) {
            return;
        }

        auto renderData = mDataProvider->getRenderData();
        if (!renderData) {
            return;
        }

        for (const auto &data: *renderData) {
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

    void RadarRender::drawArea(HDC hDC, const RenderData &data) {
        if (data.mCoordinates.size() < 3) {
            return;
        }

        std::vector<POINT> points;
        for (const auto &coord: data.mCoordinates) {
            EuroScopePlugIn::CPosition pos;
            pos.m_Latitude = coord.mLatitude;
            pos.m_Longitude = coord.mLongitude;
            POINT pt = ConvertCoordFromPositionToPixel(pos);
            points.push_back(pt);
        }

        HBRUSH hBrush = CreateSolidBrush(data.mFill);
        HBRUSH hOldBrush = (HBRUSH) SelectObject(hDC, hBrush);

        SetPolyFillMode(hDC, WINDING);
        Polygon(hDC, points.data(), static_cast<int>(points.size()));

        SelectObject(hDC, hOldBrush);
        DeleteObject(hBrush);

        if (!data.mRawColor.empty()) {
            HPEN hPen = CreatePen(PS_SOLID, 1, data.mColor);
            HPEN hOldPen = (HPEN) SelectObject(hDC, hPen);

            Polygon(hDC, points.data(), static_cast<int>(points.size()));

            SelectObject(hDC, hOldPen);
            DeleteObject(hPen);
        }
    }

    void RadarRender::drawText(HDC hDC, const RenderData &data) {
        if (data.mCoordinates.empty() || data.mText.empty()) {
            return;
        }

        const auto &coord = data.mCoordinates[0];
        EuroScopePlugIn::CPosition pos;
        pos.m_Latitude = coord.mLatitude;
        pos.m_Longitude = coord.mLongitude;
        POINT pt = ConvertCoordFromPositionToPixel(pos);

        SetTextColor(hDC, data.mColor);
        SetBkMode(hDC, TRANSPARENT);

        int fontSize = data.mFontSize > 0 ? data.mFontSize : 12;
        HFONT hFont = CreateFontW(
                -fontSize, 0, 0, 0,
                FW_NORMAL,
                FALSE, FALSE, FALSE,
                DEFAULT_CHARSET,
                OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY,
                DEFAULT_PITCH | FF_SWISS,
                L"Arial"
        );

        HFONT hOldFont = (HFONT) SelectObject(hDC, hFont);

        TextOut(hDC, pt.x, pt.y, data.mText.c_str(), static_cast<int>(data.mText.length()));

        SelectObject(hDC, hOldFont);
        DeleteObject(hFont);
    }

    bool RadarRender::OnCompileCommand(const char *sCommandLine) {
        std::string command(sCommandLine);
        if (command == ".reload") {
            mDataProvider->resetData();
            mIsLoaded = mDataProvider->loadData(mConfigPath);
            RequestRefresh();
            return true;
        }
        return false;
    }

}
