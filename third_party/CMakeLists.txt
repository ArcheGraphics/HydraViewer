#  Copyright (c) 2024 Feng Yang
#
#  I am making my contributions/submissions to this project solely in my
#  personal capacity and am not conveying any rights to any intellectual
#  property of any third parties.

# Metal cpp
add_library(metal-cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/definition.cpp
        )

target_include_directories(metal-cpp PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/metal-cpp-extensions"
)

target_link_libraries(metal-cpp
        "-framework Metal"
        "-framework MetalKit"
        "-framework AppKit"
        "-framework Foundation"
        "-framework QuartzCore"
        )

# node editor
add_subdirectory(nodeeditor)

# imgui
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/imgui")
set(IMGUI_FILES
        "${IMGUI_DIR}/imgui.cpp"
        "${IMGUI_DIR}/imgui_draw.cpp"
        "${IMGUI_DIR}/imgui_tables.cpp"
        "${IMGUI_DIR}/imgui_widgets.cpp"
        "${IMGUI_DIR}/imgui.h"
        "${IMGUI_DIR}/imgui_internal.h"
        "${IMGUI_DIR}/imstb_rectpack.h"
        "${IMGUI_DIR}/imstb_textedit.h"
        "${IMGUI_DIR}/imstb_truetype.h"
        "${IMGUI_DIR}/backends/imgui_impl_metal.h"
        "${IMGUI_DIR}/backends/imgui_impl_metal.mm"
)
add_library(imgui OBJECT ${IMGUI_FILES})
target_include_directories(imgui PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/misc/cpp)
target_compile_definitions(imgui PUBLIC IMGUI_USER_CONFIG="${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h")
target_link_libraries(imgui PRIVATE
        metal-cpp
)