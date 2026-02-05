// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include "render_data_yaml_provider.h"

namespace RenderPlugin {
    RenderDataYamlProvider::RenderDataYamlProvider() : RenderDataProvider() {}

    bool RenderDataYamlProvider::loadData(const fs::path &path) {
        if (mIsLoaded) {
            return false;
        }

        YAML::Node config = YAML::LoadFile(path.string());
        auto colorsNode = config[COLOR_KEY];
        auto featuresNode = config[FEATURE_KEY];
        if (!colorsNode || !featuresNode) {
            return false;
        }

        mColorMap = std::make_shared<ColorMap>();
        for (const auto &item: colorsNode) {
            auto key = item.first.as<std::string>();
            auto value = item.second.as<std::string>();
            if (key.empty() || value.empty()) {
                continue;
            }
            if (value.at(0) != '#') {
                continue;
            }
            mColorMap->emplace(key, Color::fromColorString(value));
        }

        // we have already written a specialized template to process the render data
        // so we can use the YAML::Node::as<T>() method to process the render data automatically
        mRenderDataVector = std::make_shared<RenderDataVector>(featuresNode.as<RenderDataVector>());
        for (auto &element: *mRenderDataVector) {
            if (element.mRawFill.empty() && element.mRawColor.empty()) {
                element.mFill = Color();
                element.mColor = Color();
                continue;
            }
            element.mFill = this->processColorField(element.mRawFill);
            element.mColor = this->processColorField(element.mRawColor);
            if (!element.mRawTextBackground.empty()) {
                element.mTextBackground = this->processColorField(element.mRawTextBackground);
            }
            if (!element.mRawTextBackgroundStroke.empty()) {
                element.mTextBackgroundStroke = this->processColorField(element.mRawTextBackgroundStroke);
            }
        }

        mIsLoaded = true;
        return true;
    }
}
