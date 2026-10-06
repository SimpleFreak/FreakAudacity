cmake_minimum_required(VERSION 4.3 FATAL_ERROR)

set(PROJECT_NAME au3-utility)
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(HEADERS
    Callable.hpp
    CRTPBase.hpp
    IteratorX.hpp
    MemoryStream.hpp
    MemoryX.hpp
    ModuleConstants.hpp
    Observer.hpp
    TypedAny.hpp
    GlobalVariable.hpp
    LockFreeQueue.hpp
    utility_api.hpp
)

set(SOURCES
    Callable.cpp
    MemoryStream.cpp
    MemoryX.cpp
    ModuleConstants.cpp
    Observer.cpp
    TypedAny.cpp
)

add_library(${PROJECT_NAME} SHARED ${HEADERS} ${SOURCES})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${HEADERS} ${SOURCES} CMakeLists.txt)
