// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include <sstream>
#include <filesystem>
#include "radar_render.h"
#include "render_data_provider.h"

using namespace Gdiplus;

namespace RenderPlugin {
    RadarRender::RadarRender(ProviderPtr dataProvider, fs::path configPath) : mDataProvider(std::move(dataProvider)),
                                                                              mConfigPath(std::move(configPath)),
                                                                              mIsLoaded(false) {
        GdiplusStartup(&mGdiplusToken, &mGdiplusStartupInput, nullptr);
        mIsLoaded = mDataProvider->loadData(mConfigPath);
    }

    RadarRender::~RadarRender() {
        if (mIsLoaded) {
            mDataProvider->resetData();
            mDataProvider.reset();
            mIsLoaded = false;
            GdiplusShutdown(mGdiplusToken);
        }
    }

    void RadarRender::OnRefresh(HDC hDC, int Phase) {
        if (!mIsLoaded || Phase != EuroScopePlugIn::REFRESH_PHASE_BACK_BITMAP) {
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

    void RadarRender::drawLine(HDC hDC, const RenderData &data) {
        if (data.mCoordinates.size() < 2) {
            return;
        }

        std::vector<Point> points;
        for (const auto &coord: data.mCoordinates) {
            POINT pt = ConvertCoordFromPositionToPixel(coord.toPosition());
            points.emplace_back(pt.x, pt.y);
        }

        Graphics graphics(hDC);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        Pen pen(data.mColor, 1.0f);

        graphics.DrawLines(&pen, points.data(), static_cast<int>(points.size()));
    }

    void RadarRender::drawArea(HDC hDC, const RenderData &data) {
        if (data.mCoordinates.size() < 3) {
            return;
        }

        std::vector<Point> points;
        for (const auto &coord: data.mCoordinates) {
            POINT pt = ConvertCoordFromPositionToPixel(coord.toPosition());
            points.emplace_back(pt.x, pt.y);
        }

        Graphics graphics(hDC);
        graphics.SetSmoothingMode(SmoothingModeAntiAlias);
        Pen pen(data.mColor, 1.0f);
        SolidBrush brush(data.mFill);

        graphics.FillPolygon(&brush, points.data(), static_cast<int>(points.size()));
        if (!data.mRawColor.empty()) {
            graphics.DrawPolygon(&pen, points.data(), static_cast<int>(points.size()));
        }
    }

    void RadarRender::drawText(HDC hDC, const RenderData &data) {
        if (data.mCoordinates.empty() || data.mText.empty()) {
            return;
        }

        const auto &coord = data.mCoordinates[0];
        POINT pt = ConvertCoordFromPositionToPixel(coord.toPosition());
        PointF point = PointF(pt.x, pt.y);

        Graphics graphics(hDC);
        graphics.SetTextRenderingHint(TextRenderingHintAntiAliasGridFit);
        SolidBrush brush(data.mColor);
        Font font(L"Euroscope", data.mFontSize, FontStyleRegular, UnitPixel);
        graphics.DrawString(data.mText.c_str(), -1, &font, point,&brush);
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

    void RadarRender::OnAsrContentToBeClosed() {

    }
}
