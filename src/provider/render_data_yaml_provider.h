// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#ifndef RENDERPLUGIN_RENDER_DATA_YAML_PROVIDER_H
#define RENDERPLUGIN_RENDER_DATA_YAML_PROVIDER_H

#include "render_data_provider.h"
#include "string_utils.h"
#include <yaml-cpp/yaml.h>

namespace RenderPlugin {
    class RenderDataYamlProvider : public RenderDataProvider {
    public:
        RenderDataYamlProvider();

        virtual bool loadData(const fs::path &path) override;
    };
}

namespace YAML {
    template<>
    struct convert<RenderPlugin::Coordinate> {
        static Node encode(const RenderPlugin::Coordinate &rhs) {
            Node node;
            node.push_back(rhs.mLongitude);
            node.push_back(rhs.mLatitude);
            return node;
        }

        static bool decode(const Node &node, RenderPlugin::Coordinate &rhs) {
            if (!node.IsSequence() || node.size() != 2) {
                return false;
            }

            rhs.mLongitude = node[0].as<double>();
            rhs.mLatitude = node[1].as<double>();
            return true;
        }
    };

    template<>
    struct convert<RenderPlugin::RenderData> {
        static Node encode(const RenderPlugin::RenderData &rhs) {
            Node node;
            node["type"] = RenderPlugin::renderTypeToString(rhs.mType);
            node["coordinates"] = rhs.mCoordinates;
            // area type support fill (area fill color)
            if (!rhs.mRawFill.empty()) {
                node["fill"] = rhs.mRawFill;
            }
            // line/area/text type support color (line color or text color)
            if (!rhs.mRawColor.empty()) {
                node["color"] = rhs.mRawColor;
            }
            // text type support text content
            if (!rhs.mText.empty()) {
                node["text"] = RenderPlugin::WstringToUtf8(rhs.mText);
            }
            // text type support font size
            if (rhs.mFontSize > 0) {
                node["size"] = rhs.mFontSize;
            }
            node["zoom"] = rhs.mZoom;
            node["stroke"] = RenderPlugin::lineStyleToString(rhs.mLineStyle);
            if (rhs.mStrokeWidth > 0.0f) node["strokeWidth"] = rhs.mStrokeWidth;
            if (rhs.mDashLength > 0.0f || rhs.mGapLength > 0.0f) {
                node["dashLength"] = rhs.mDashLength;
                node["gapLength"] = rhs.mGapLength;
            }
            return node;
        }

        static bool decode(const Node &node, RenderPlugin::RenderData &rhs) {
            if (!node.IsMap()) {
                return false;
            }
            rhs.mType = RenderPlugin::stringToRenderType(node["type"].as<std::string>());
            rhs.mCoordinates = node["coordinates"].as<RenderPlugin::Coordinates>();
            if (node["fill"]) {
                rhs.mRawFill = node["fill"].as<std::string>();
            }
            if (node["color"]) {
                rhs.mRawColor = node["color"].as<std::string>();
            }
            if (node["text"]) {
                rhs.mText = RenderPlugin::Utf8ToWstring(node["text"].as<std::string>());
            }
            if (node["size"]) {
                rhs.mFontSize = node["size"].as<int>();
            }
            if (node["zoom"]) {
                rhs.mZoom = node["zoom"].as<int>();
            }
            if (node["stroke"]) {
                rhs.mLineStyle = RenderPlugin::stringToLineStyle(node["stroke"].as<std::string>());
            } else if (node["lineStyle"]) {
                rhs.mLineStyle = RenderPlugin::stringToLineStyle(node["lineStyle"].as<std::string>());
            }
            if (node["strokeWidth"]) {
                rhs.mStrokeWidth = node["strokeWidth"].as<float>();
            }
            if (node["dash"]) {
                auto dash = node["dash"];
                if (dash.IsSequence() && dash.size() >= 2) {
                    rhs.mDashLength = dash[0].as<float>();
                    rhs.mGapLength = dash[1].as<float>();
                }
            } else {
                if (node["dashLength"]) rhs.mDashLength = node["dashLength"].as<float>();
                if (node["gapLength"]) rhs.mGapLength = node["gapLength"].as<float>();
            }
            return true;
        }
    };
}

#endif
