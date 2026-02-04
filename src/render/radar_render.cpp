// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include <sstream>
#include <filesystem>
#include "radar_render.h"
#include "render_data_provider.h"

namespace RenderPlugin {
    RadarRender::RadarRender(ProviderPtr dataProvider,
                             RenderPtr render,
                             fs::path configPath) : mDataProvider(std::move(dataProvider)),
                                                    mRender(std::move(render)),
                                                    mConfigPath(std::move(configPath)),
                                                    mIsLoaded(false) {
        mIsLoaded = mDataProvider->loadData(mConfigPath);
    }

    RadarRender::~RadarRender() {
        if (mIsLoaded) {
            mDataProvider->resetData();
            mDataProvider.reset();
            mIsLoaded = false;
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
