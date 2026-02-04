// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_RENDER_DATA_PROVIDER_H
#define RENDERPLUGIN_RENDER_DATA_PROVIDER_H

#include <filesystem>
#include "render_data_definition.hpp"

namespace RenderPlugin {
    namespace fs = std::filesystem;

    class RenderDataProvider {
    public:
        RenderDataProvider();

        virtual ~RenderDataProvider();

        virtual bool loadData(const fs::path &path) = 0;

        Color getColor(const std::string &name);

        std::shared_ptr<RenderDataVector> getRenderData();

        void resetData();

    protected:
        bool mIsLoaded;
        std::shared_ptr<ColorMap> mColorMap;
        std::shared_ptr<RenderDataVector> mRenderDataVector;

        Color processColorField(const std::string &rawColor);
    };

    using ProviderPtr = std::unique_ptr<RenderDataProvider>;
}

#endif
