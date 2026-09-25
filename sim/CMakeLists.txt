# Mini Jet Boat Dash — simulator build (native SDL2 or Emscripten/WebAssembly)
#
# Browser: emcmake cmake -S sim -B build-web && cmake --build build-web
# Native:  cmake -S sim -B build && cmake --build build   (needs SDL2)

cmake_minimum_required(VERSION 3.20)
project(dash_sim C)

set(CMAKE_C_STANDARD 11)
set(REPO_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)

# ---------- LVGL (pinned) ----------
include(FetchContent)
FetchContent_Declare(lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG        v9.6.0
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  do_not_add_lvgl_cmake)   # we compile the sources ourselves
FetchContent_MakeAvailable(lvgl)

file(GLOB_RECURSE LVGL_SOURCES CONFIGURE_DEPENDS ${lvgl_SOURCE_DIR}/src/*.c)
add_library(lvgl STATIC ${LVGL_SOURCES})
target_include_directories(lvgl PUBLIC ${lvgl_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(lvgl PUBLIC LV_CONF_INCLUDE_SIMPLE)

# ---------- SDL2 ----------
if(EMSCRIPTEN)
    target_compile_options(lvgl PUBLIC -sUSE_SDL=2)
    target_link_options(lvgl PUBLIC -sUSE_SDL=2)
else()
    find_package(SDL2 REQUIRED)
    target_include_directories(lvgl PUBLIC ${SDL2_INCLUDE_DIRS} ${SDL2_INCLUDE_DIRS}/..)
    target_link_libraries(lvgl PUBLIC ${SDL2_LIBRARIES} m)
endif()

# ---------- Stencil image (assets/ -> C) ----------
find_package(Python3 REQUIRED COMPONENTS Interpreter)
file(GLOB STENCIL_FILES CONFIGURE_DEPENDS ${REPO_ROOT}/assets/stencil*)
add_custom_command(
    OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/stencil_img.c
    COMMAND ${Python3_EXECUTABLE} ${REPO_ROOT}/tools/stencil_to_c.py ${CMAKE_CURRENT_BINARY_DIR}/stencil_img.c
    DEPENDS ${REPO_ROOT}/tools/stencil_to_c.py ${STENCIL_FILES}
    COMMENT "Embedding stencil image")

# ---------- Dash + simulator ----------
file(GLOB_RECURSE DASH_SOURCES CONFIGURE_DEPENDS ${REPO_ROOT}/src/*.c)
add_executable(dash_sim
    main.c
    fake_ecu.c
    sim_overlay.c
    ${DASH_SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/stencil_img.c)
target_link_libraries(dash_sim PRIVATE lvgl)
target_compile_options(dash_sim PRIVATE -Wall -Wextra -Wno-unused-parameter)

if(EMSCRIPTEN)
    set_target_properties(dash_sim PROPERTIES OUTPUT_NAME index SUFFIX ".html")
    target_link_options(dash_sim PRIVATE
        -sALLOW_MEMORY_GROWTH=1
        -sINITIAL_MEMORY=67108864
        --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)
    set_property(TARGET dash_sim APPEND PROPERTY LINK_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/web/shell.html)
endif()
