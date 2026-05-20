cmake_minimum_required(VERSION 3.20)
project(Watermelon_Game C)
set(CMAKE_C_STANDARD 11)

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
        raylib
        X11
        m
        dl
        pthread
)

target_link_options(Watermelon_Game PRIVATE
        -Wl,--allow-multiple-definition
)
file(COPY ${CMAKE_SOURCE_DIR}/Resources DESTINATION ${CMAKE_BINARY_DIR})
