cmake_minimum_required(VERSION 3.20)

if(MINGW)
    set(CMAKE_RC_COMPILER windres)
    set(CMAKE_RC_FLAGS "")
endif()

project(Watermelon_Game C RC)

set(CMAKE_C_STANDARD 11)

set(SRC_DIR     ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

file(GLOB SRCS_JOGO "${SRC_DIR}/*.c")

# Raylib via submodule
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(raylib)

# Chipmunk2D via submodule
set(BUILD_DEMOS   OFF CACHE BOOL "" FORCE)
set(INSTALL_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED  OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC  ON  CACHE BOOL "" FORCE)
add_subdirectory(chipmunk)

# Ícone apenas no Windows
if(MINGW)
    set(RC_FILE resources.rc)
endif()

add_executable(Watermelon_Game
    ${SRCS_JOGO}
    ${RC_FILE}
)

target_include_directories(Watermelon_Game PRIVATE
    ${INCLUDE_DIR}
    raylib/src
    chipmunk/include
)

target_link_libraries(Watermelon_Game PRIVATE
    raylib
    chipmunk_static
    m
)

target_link_options(Watermelon_Game PRIVATE
    -Wl,--allow-multiple-definition
)

add_custom_command(TARGET Watermelon_Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Resources
    $<TARGET_FILE_DIR:Watermelon_Game>/Resources
)
