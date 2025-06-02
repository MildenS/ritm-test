PROJECT(RITM-TEST)

cmake_minimum_required(VERSION 3.16)

add_library(GENERATOR_LIB src/parser.cpp)

target_include_directories(GENERATOR_LIB PUBLIC include)

add_subdirectory(tinyxml2)


add_executable(${PROJECT_NAME} src/main.cpp)

target_link_libraries(${PROJECT_NAME} GENERATOR_LIB)
