cmake_minimum_required (VERSION 3.10)
project (Puzzle-Solver)

set(CMAKE_EXPORT_COMPILE_COMMANDS "on")

option(BUILD_EXPERIMENTAL "Build experimental tools" OFF)
option(USE_FINDSDL2 "Use the FindSDL2.cmake script" OFF)

option(PUZZLE_SOLVER_SANITIZE_THREAD         "puzzle-solver: enable thread sanitizer" OFF)
option(PUZZLE_SOLVER_SANITIZE_ADDRESS        "puzzle-solver: enable address sanitizer" OFF)
option(PUZZLE_SOLVER_SANITIZE_UNDEFINED      "puzzle-solver: enable undefined sanitizer" OFF)

# sanitizers

if (PUZZLE_SOLVER_SANITIZE_THREAD)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=thread")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread")
endif()

if (PUZZLE_SOLVER_SANITIZE_ADDRESS)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
endif()

if (PUZZLE_SOLVER_SANITIZE_UNDEFINED)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=undefined")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined")
endif()


set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
if (USE_FINDSDL2)
    set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2")
endif()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RELEASE CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "DEBUG" "RELEASE" "RELWITHDEBINFO")
endif()

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(SDL2)

if (NOT USE_FINDSDL2 AND NOT SDL2_FOUND)
    message(WARNING "Unable to find SDL2 library. It is either not installed or CMake cannot find it."
        " In the latter case, setting the USE_FINDSDL2 variable might help:\n"
        "   $ cmake -D USE_FINDSDL2 .."
        )

    message(FATAL_ERROR "Aborting")
endif()

string(STRIP "${SDL2_LIBRARIES}" SDL2_LIBRARIES)

if (APPLE)
    find_library(COREGRAPHICS_LIBRARY CoreGraphics )
    find_library(COREFOUNDATION_LIBRARY CoreFoundation )
else (APPLE)
    unset(COREGRAPHICS_LIBRARY)
    unset(COREFOUNDATION_LIBRARY)
endif (APPLE)

include_directories(imgui)
include_directories(imgui/examples)
include_directories(imgui/examples/libs/gl3w)
include_directories(${SDL2_INCLUDE_DIRS})

add_library(Core STATIC
    BufferRGB.cpp
    ImageRGB.cpp
    StateApp.cpp
    LoadedImage.cpp
    ViewLoadedImages.cpp
    ViewSelectedImage.cpp
    ViewActions.cpp
    CommonPoint.cpp
    Homography.cpp
    )

target_include_directories(Core PUBLIC
    .
    )

target_link_libraries(Core PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
    ${COREFOUNDATION_LIBRARY}
    )

add_library(CoreGL STATIC
    Texture.cpp
    ImageRGBWithTexture.cpp
    )

target_include_directories(CoreGL PUBLIC
    .
    )

target_link_libraries(CoreGL PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
    ${SDL2_LIBRARIES}
    ${COREFOUNDATION_LIBRARY}
    )

add_library(ImGui STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_widgets.cpp
    imgui/examples/libs/gl3w/GL/gl3w.c
    imgui/examples/imgui_impl_sdl.cpp
    imgui/examples/imgui_impl_opengl3.cpp
    )

target_compile_definitions(ImGui PRIVATE
    IMGUI_USER_CONFIG="../imconfig-vtx32.h"
    )

target_link_libraries(ImGui PRIVATE
    ${CMAKE_DL_LIBS}
    ${SDL2_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${COREFOUNDATION_LIBRARY}
    )

add_executable(main main.cpp)
target_link_libraries(main PRIVATE Core CoreGL ImGui)

add_executable(diff diff.cpp)
target_link_libraries(diff PRIVATE Core)

add_executable(align align.cpp)
target_link_libraries(align PRIVATE Core)

#
## Experimental stuff

if (BUILD_EXPERIMENTAL)
endif()
