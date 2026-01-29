// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include <fstream>
#include "render_data_provider.h"
#include "color_utils.h"

namespace RenderPlugin {
    RenderDataProvider::RenderDataProvider() : mColorMap(nullptr), mRenderDataVector(nullptr), mIsLoaded(false) {}

    RenderDataProvider::~RenderDataProvider() {
        if (mIsLoaded) {
            mColorMap.reset();
            mRenderDataVector.reset();
            mIsLoaded = false;
        }
    }

    COLORREF RenderDataProvider::getColor(const std::string &name) {
        if (!mIsLoaded) {
            return DEFAULT_COLOR;
        }
        if (mColorMap->find(name) == mColorMap->end()) {
            return DEFAULT_COLOR;
        }
        return mColorMap->at(name);
    }

    std::shared_ptr<RenderDataVector> RenderDataProvider::getRenderData() {
        return mRenderDataVector;
    }

    void RenderDataProvider::resetData() {
        mColorMap.reset();
        mRenderDataVector.reset();
        mIsLoaded = false;
    }

    COLORREF RenderDataProvider::processColorField(const std::string &rawColor) {
        if (rawColor.empty()) {
            // if the color field is empty, we use the default color
            return DEFAULT_COLOR;
        }
        if (rawColor.at(0) != '#') {
            // if the color value wasn't start with '#', it means that it's a color name
            // so we get the color from the color map
            return this->getColor(rawColor);
        }
        // else we parse the color
        return parseColor(rawColor);
    }
}
