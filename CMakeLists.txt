cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(PROJECT_NAME Audacity)
project(${PROJECT_NAME} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

include_directories(D:/Qt/6.11.2/msvc2022_64/include)
include_directories(D:/Qt/6.11.2/Src)

if (MSVC)
    # Now that the `audacity` target is defined, we can set it as the startup project in Visual Studio.
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    src/app/main.cpp src/app/main_window.hpp src/app/main_window.cpp src/app/resource_monitor_windows.hpp ci_build.cmake SetupConfigurare.cmake
)

add_executable(${PROJECT_NAME}
    src/app/main.cpp src/app/main_window.hpp src/app/main_window.cpp src/app/resource_monitor_windows.hpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    psapi
    kernel32
)
