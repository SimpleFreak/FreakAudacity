cmake_minimum_required(VERSION 4.3 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

find_package(Catch2 3 REQUIRED)

function(add_unit_test)
    cmake_parse_arguments(ARG "" "NAME" "SOURCES;LIBRARIES" ${ARGN})

    add_executable(${ARG_NAME} ${ARG_SOURCES})

    target_link_libraries(${ARG_NAME} PRIVATE ${ARG_LIBRARIES} Catch2::Catch2WithMain)

    add_test(NAME ${ARG_NAME} COMMAND ${ARG_NAME})
endfunction()

set(PROJECT_NAME au3-utility_test)
project(${PROJECT_NAME})

add_library(au3-utility STATIC
    ../Callable.hpp
    ../Callable.cpp
)

add_unit_test(
    NAME
    ${PROJECT_NAME}
    SOURCES
    CallableTest.cpp
    LIBRARIES
    au3-utility
)
