#  Copyright (c) 2024 Feng Yang
#
#  I am making my contributions/submissions to this project solely in my
#  personal capacity and am not conveying any rights to any intellectual
#  property of any third parties.

cmake_minimum_required(VERSION 3.16)

project(editor LANGUAGES C CXX)

set(COMMON_FILES
        swapchain.h
        swapchain.cpp
        swapchain_layer.mm
        camera.h
        camera.cpp
        viewport.h
        viewport.mm
        framerate.h
        framerate.cpp
        common.h
        common.cpp
        # model
        model/root_data_model.h
        model/root_data_model.cpp
        model/selection_data_model.h
        model/selection_data_model.cpp
        model/view_settings_data_model.h
        model/view_settings_data_model.cpp
        model/free_camera.h
        model/free_camera.cpp
        model/custom_attributes.h
        model/custom_attributes.cpp
)

source_group("common\\" FILES ${COMMON_FILES})

set(PROJECT_FILES
        ${COMMON_FILES}
        stage_view.cpp
        stage_view.h
)

add_library(${PROJECT_NAME} OBJECT ${PROJECT_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../)

# Link third party libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt::Core
        Qt::Widgets
        fmt::fmt
        spdlog::spdlog
        metal-cpp
        common
)