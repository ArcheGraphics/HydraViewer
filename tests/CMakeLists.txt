#  Copyright (c) 2024 Feng Yang
#
#  I am making my contributions/submissions to this project solely in my
#  personal capacity and am not conveying any rights to any intellectual
#  property of any third parties.

cmake_minimum_required(VERSION 3.16)
project(tests LANGUAGES CXX)

qt_standard_project_setup()

qt_add_executable(${PROJECT_NAME}
        main.cpp
        test_qstring.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Test
        Qt6::Widgets
)