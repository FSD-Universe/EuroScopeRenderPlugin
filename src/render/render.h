// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_RENDER_H
#define RENDERPLUGIN_RENDER_H

#include <memory>
#include <render_data_definition.hpp>

namespace RenderPlugin {
    class Render {
    public:
        Render() = default;

        virtual ~Render() = default;

        virtual void drawLine(HDC hdc, const std::vector<POINT> &points, const RenderData &data) = 0;

        virtual void drawArea(HDC hdc, const std::vector<POINT> &points, const RenderData &data) = 0;

        virtual void drawText(HDC hdc, const POINT &pt, const RenderData &data) = 0;
    };

    using RenderPtr = std::unique_ptr<Render>;
}

#endif
