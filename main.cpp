// Copyright (c) 2026 Half_nothing
// SPDX-License-Identifier: MIT

#include <fstream>
#include "euroscope_render_plugin.h"
#include "render_data_yaml_provider.h"

namespace fs = std::filesystem;

RenderPlugin::EuroScopeRenderPlugin *pPlugInInstance = nullptr;
HMODULE pluginModule = nullptr;
std::shared_ptr<std::ofstream> logStream;

inline std::string getRealFileName(const std::string &path);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            pluginModule = hModule;
            break;
    }
    return TRUE;
}

void __declspec (dllexport) EuroScopePlugInInit(EuroScopePlugIn::CPlugIn **ppPlugInInstance) {
    logStream = std::make_shared<std::ofstream>(getRealFileName("log.txt"), std::ios::out | std::ios::app);
    *logStream << "EuroScopePlugInInit" << std::endl;
    auto provider = std::make_unique<RenderPlugin::RenderDataYamlProvider>();
    *logStream << "EuroScopePlugInInit: provider created" << std::endl;
    *logStream << "EuroScopePlugInInit: config path: " << getRealFileName("config.yaml") << std::endl;
    auto render = std::make_unique<RenderPlugin::RadarRender>(std::move(provider), getRealFileName("config.yaml"));
    *logStream << "EuroScopePlugInInit: render created" << std::endl;
    *ppPlugInInstance = pPlugInInstance = new RenderPlugin::EuroScopeRenderPlugin(std::move(render));
    *logStream << "EuroScopePlugInInit: plugin created" << std::endl;
}

void __declspec (dllexport) EuroScopePlugInExit() {
    *logStream << "EuroScopePlugInExit" << std::endl;
    logStream->close();
    logStream.reset();
    if (pPlugInInstance) {
        delete pPlugInInstance;
        pPlugInInstance = nullptr;
    }
}

inline std::string getRealFileName(const std::string &path) {
    namespace fs = std::filesystem;
    if (path.empty()) {
        return path;
    }
    fs::path pFilename = path;
    if (!fs::is_regular_file(pFilename) && pluginModule != nullptr) {
        TCHAR pBuffer[MAX_PATH] = {0};
        GetModuleFileName(pluginModule, pBuffer, sizeof(pBuffer) / sizeof(TCHAR) - 1);
        fs::path dllPath = pBuffer;
        pFilename = dllPath.parent_path() / path;
    }
    return pFilename.string();
}

//int main() {
//    auto provider = std::make_unique<RenderPlugin::RenderDataYamlProvider>();
//    auto render = std::make_unique<RenderPlugin::RadarRender>(std::move(provider), "config.yaml");
//    return 0;
//}
