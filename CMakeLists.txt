cmake_minimum_required(VERSION 3.31)
project(RenderPlugin LANGUAGES CXX VERSION 1.0.0)

message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "VCPKG_TARGET_TRIPLET: ${VCPKG_TARGET_TRIPLET}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
set(CMAKE_GENERATOR_PLATFORM "Win32")
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

find_package(yaml-cpp CONFIG REQUIRED)

message("Build Type: ${CMAKE_BUILD_TYPE} ${CMAKE_CXX_FLAGS}")
message("PROJECT_NAME: ${PROJECT_NAME}")
message("PROJECT_VERSION: ${PROJECT_VERSION}")
message("PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")
message("PROJECT_BINARY_DIR: ${PROJECT_BINARY_DIR}")

include_directories(
        include
        src/plugin
        src/provider
        src/render
        src/utils
)
link_directories("${PROJECT_SOURCE_DIR}/lib")

include(files.cmake)

#add_executable(${PROJECT_NAME} ${SOURCE_FILE})
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILE})

target_compile_definitions(${PROJECT_NAME} PRIVATE _X86_ WIN32 _WINDOWS YAML_CPP_STATIC_DEFINE)

target_link_libraries(${PROJECT_NAME} PRIVATE EuroScopePlugIn yaml-cpp::yaml-cpp gdiplus d2d1 dwrite)
