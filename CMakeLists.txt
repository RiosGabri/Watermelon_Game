cmake_minimum_required(VERSION 3.20)
project(Watermelon_Game C)

set(CMAKE_C_STANDARD 11)
add_definitions(-D__USE_MINGW_ANSI_STDIO=1)

set(SRC_DIR     ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

file(GLOB SRCS_JOGO     "${SRC_DIR}/*.c")
file(GLOB SRCS_CHIPMUNK "${SRC_DIR}/chipmunk/*.c")

add_executable(Watermelon_Game
        ${SRCS_JOGO}
        ${SRCS_CHIPMUNK}
)

target_include_directories(Watermelon_Game PRIVATE ${INCLUDE_DIR})

target_link_libraries(Watermelon_Game PRIVATE
        C:/msys64/ucrt64/lib/libraylib.a
        C:/msys64/ucrt64/lib/libglfw3.dll.a
        opengl32
        gdi32
        winmm
        m
)

target_link_options(Watermelon_Game PRIVATE
        -Wl,--allow-multiple-definition
        -L"C:/msys64/ucrt64/lib"
)