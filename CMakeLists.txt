cmake_minimum_required(VERSION 3.27)
project(HydraViewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Add path for local cmake scripts
list(APPEND CMAKE_MODULE_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/../USDInstall)

# usd
include(pxrConfig)
include_directories(${PXR_INCLUDE_DIRS})
set(PXR_LIBRARY_NAMES)
foreach (_item ${PXR_LIBRARIES}) # Iterate over list of absolute paths
    get_filename_component(_t ${_item} NAME)
    list(APPEND PXR_LIBRARY_NAMES ${_t})
endforeach ()
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

set(CORE_FILES
        main.cpp
)

add_executable(HydraViewer ${CORE_FILES})
target_link_libraries(HydraViewer
        Qt::Core
        Qt::Widgets
        metal-cpp
        common
        viewport
        ${PXR_LIBRARY_NAMES}
)

