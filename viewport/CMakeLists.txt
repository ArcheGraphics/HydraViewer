#  Copyright (c) 2023 Feng Yang
#
#  I am making my contributions/submissions to this project solely in my
#  personal capacity and am not conveying any rights to any intellectual
#  property of any third parties.

cmake_minimum_required(VERSION 3.16)

project(viewport LANGUAGES C CXX)

set(COMMON_FILES
        swapchain.h
        swapchain.cpp
        swapchain_layer.mm
        camera.h
        camera.cpp
        viewport.h
        viewport.mm
)

source_group("common\\" FILES ${COMMON_FILES})

set(PROJECT_FILES
        ${COMMON_FILES}
)

#NB: switch this to shared library and things stop working. (there is likely two copies of volk somewhere.
add_library(${PROJECT_NAME} STATIC ${PROJECT_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../)

# Link third party libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
        fmt::fmt
        spdlog::spdlog
        metal-cpp
        common
)