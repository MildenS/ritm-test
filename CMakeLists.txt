PROJECT(RITM-TEST)

cmake_minimum_required(VERSION 3.16)

set (CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(GENERATOR_LIB src/parser.cpp
                          src/generator.cpp)

target_include_directories(GENERATOR_LIB PUBLIC include)

add_subdirectory(tinyxml2)

target_link_libraries(GENERATOR_LIB tinyxml2)

add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} GENERATOR_LIB)
