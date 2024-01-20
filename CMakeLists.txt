cmake_minimum_required(VERSION 3.27)
project(HydraViewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH /Users/yangfeng/Qt/6.6.1/macos/lib/cmake)

# Add path for local cmake scripts
list(APPEND CMAKE_MODULE_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/../USDInstall)

# usd
include(pxrConfig)
include_directories(${PXR_INCLUDE_DIRS})

if (APPLE)
    add_definitions(-DPXR_METAL_SUPPORT_ENABLED)
    add_compile_definitions(BOOST_NO_CXX98_FUNCTION_BASE)
endif ()

find_package(Qt6 COMPONENTS
        Core Widgets
        REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_subdirectory(third_party)
add_subdirectory(common)
add_subdirectory(viewport)
add_subdirectory(app)
