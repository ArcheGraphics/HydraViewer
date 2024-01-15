cmake_minimum_required(VERSION 3.27)
project(HydraViewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt6 COMPONENTS
        Core Widgets
        REQUIRED)

add_executable(HydraViewer main.cpp)
target_link_libraries(HydraViewer
        Qt::Core
        Qt::Widgets
)

