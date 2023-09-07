cmake_minimum_required(VERSION 3.4)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall")

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(src) 

target_link_libraries(${PROJECT_NAME} src)