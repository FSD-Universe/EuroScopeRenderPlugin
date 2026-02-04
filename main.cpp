// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include "euroscope_render_plugin.h"
#include "render_data_yaml_provider.h"
#include "direct2d_render.h"

namespace fs = std::filesystem;

RenderPlugin::EuroScopeRenderPlugin *pPlugInInstance = nullptr;
HMODULE pluginModule = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            pluginModule = hModule;
            break;
    }
    return TRUE;
}

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn **ppPlugInInstance) {
    *ppPlugInInstance = pPlugInInstance = new RenderPlugin::EuroScopeRenderPlugin(pluginModule);
}

void __declspec (dllexport) EuroScopePlugInExit() {
    if (pPlugInInstance) {
        delete pPlugInInstance;
        pPlugInInstance = nullptr;
    }
}

//int main() {
//    new RenderPlugin::EuroScopeRenderPlugin(pluginModule);
//    return 0;
//}
