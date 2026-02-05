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

        /** 开始一帧绘制（可选）。Direct2D 在此绑定 DC 并 BeginDraw，每帧只调用一次。返回 false 时本帧不绘制。 */
        virtual bool beginFrame(HDC hdc) { return true; }

        /** 结束一帧绘制（可选）。Direct2D 在此 EndDraw。 */
        virtual void endFrame() {}

        virtual void drawLine(HDC hdc, const std::vector<POINT> &points, const RenderData &data) = 0;

        virtual void drawArea(HDC hdc, const std::vector<POINT> &points, const RenderData &data) = 0;

        // effectiveFontSizePixels: 按缩放换算后的字体大小（像素），<=0 时使用 data.mFontSize
        virtual void drawText(HDC hdc, const POINT &pt, const RenderData &data,
                             float effectiveFontSizePixels = 0.0f) = 0;
    };

    using RenderPtr = std::shared_ptr<Render>;
}

#endif
