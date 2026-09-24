cmake_minimum_required(VERSION 3.24 FATAL_ERROR)

set(PROJECT_NAME Audacity)
project(${PROJECT_NAME} LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(muse)

if (MSVC)
    # Now that the `audacity` target is defined, we can set it as the startup project in Visual Studio.
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    src/app/main.cpp
    muse/framework/global/thirdparty/kors_async/async/asyncable.hpp
    src/audio/audio_configuration_types.hpp
    muse/framework/global/thirdparty/kors_async/async/config.hpp
)

add_executable(${PROJECT_NAME}
    src/app/main.cpp
    muse/framework/global/thirdparty/kors_async/async/asyncable.hpp
    src/audio/audio_configuration_types.hpp
    muse/framework/global/thirdparty/kors_async/async/config.hpp
)
