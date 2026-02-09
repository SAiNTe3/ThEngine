# CMake最低版本号要求
cmake_minimum_required(VERSION 3.12)

project(OpenGL_01)
set(CMAKE_CXX_STANDARD 17)
aux_source_directory(. SOURCES)

file(GLOB RESOURCE "./Assets")

file(COPY ${RESOURCE} DESTINATION ${CMAKE_BINARY_DIR})

file(GLOB SRC_FILES "src/ESL/*.cpp" "src/ESL/BasicShape/*.cpp" "src/ESL/Widget/*.cpp" "src/game/*.cpp")


include_directories(
    SYSTEM ${CMAKE_SOURCE_DIR}/include
    SYSTEM ${CMAKE_SOURCE_DIR}/include/ESL
    SYSTEM ${CMAKE_SOURCE_DIR}/include/ESL/BasicShape
    SYSTEM ${CMAKE_SOURCE_DIR}/include/ESL/Widget
    SYSTEM ${CMAKE_SOURCE_DIR}/include/game
    SYSTEM ${CMAKE_SOURCE_DIR}/include/freetype
)
# 添加库文件路径
link_directories(
    SYSTEM ${CMAKE_SOURCE_DIR}/lib
)

add_library(glad ${CMAKE_SOURCE_DIR}/src/glad/glad.c)
add_library(esl ${SRC_FILES})

find_package(OpenGL REQUIRED)
find_package(freetype REQUIRED)
add_executable(OpenGL_test ${SOURCES} ${SRC_FILES})

target_link_libraries(OpenGL_test glfw3 ${OPENGL_LIBRARIES} glad esl freetype)
