#  Copyright (c) 2024 Feng Yang
#
#  I am making my contributions/submissions to this project solely in my
#  personal capacity and am not conveying any rights to any intellectual
#  property of any third parties.

cmake_minimum_required(VERSION 3.16)

project(editor LANGUAGES C CXX)

set(COMMON_FILES
        common.h
        common.cpp
        framerate.h
        framerate.cpp
        stage_view.h
        stage_view.cpp
        windows.h
        windows.cpp
        panels/stage_tree.h
        panels/stage_tree.cpp
        panels/render_settings.h
        panels/render_settings.cpp
        panels/view_settings_view.h
        panels/view_settings_view.cpp
        # node
        node/graph_model.h
        node/graph_model.cpp
        # viewport
        viewport/swapchain.h
        viewport/swapchain.cpp
        viewport/swapchain_layer.mm
        viewport/camera.h
        viewport/camera.cpp
        viewport/viewport.h
        viewport/viewport.mm
        # model
        model/data_model.h
        model/data_model.cpp
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
)

add_library(${PROJECT_NAME} OBJECT ${PROJECT_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ../)

# Link third party libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt::Core
        Qt::Widgets
        fmt::fmt
        metal-cpp
        QtNodes
        imgui
)

target_compile_definitions(
        ${PROJECT_NAME} PRIVATE PROJECT_PATH="${CMAKE_CURRENT_SOURCE_DIR}/..")