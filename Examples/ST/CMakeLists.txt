cmake_minimum_required(VERSION 3.13)
project(ST)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)

file(GLOB SOURCE_FILES
    "include/*.h"
    "include/*.hpp"
    "src/*.cpp" 
    "imgui/*.h"
    "imgui/*.hpp"
    "imgui/*.cpp"
)

if(MSVC)
    set(LIB_DIRS ../../build/Debug)
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(LIB_DIRS ../../build)
endif()

set(INC_DIRS ../../include ../../imgui ../../glew/include ../../glfw/include)

include_directories(${INC_DIRS})
link_directories(${LIB_DIRS})

add_executable(ST ${SOURCE_FILES})
target_link_libraries(ST PUBLIC ImGuiLoop)

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(ST PUBLIC GL GLU)
endif()

if(MSVC)
    if(NOT (MSVC_VERSION LESS 1900))
        target_link_libraries(ST PUBLIC legacy_stdio_definitions)
    endif()
    message(STATUS "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE /NODEFAULTLIB:MSVCRT /NODEFAULTLIB:LIBCMT /DEFAULTLIB:opengl32.lib")
endif()

