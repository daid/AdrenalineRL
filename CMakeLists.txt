cmake_minimum_required(VERSION 3.12)
project(AdrenalineRL)

file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

find_package(librouge REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} PUBLIC librouge-core librouge-ui)
