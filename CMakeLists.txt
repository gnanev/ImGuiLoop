cmake_minimum_required(VERSION 3.13)
project(ImGuiLoop)

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

set(INC_DIRS include imgui glew/include glfw/include)
set(BIN_DIR ${CMAKE_HOME_DIRECTORY}/glew/bin)

include_directories(${INC_DIRS})

add_definitions(-DGLEW_STATIC)

add_library(ImGuiLoop STATIC ${SOURCE_FILES})

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set_target_properties(ImGuiLoop PROPERTIES SUFFIX ".stat.a")
    add_custom_command( TARGET ImGuiLoop POST_BUILD
                        COMMAND rm build/libImGuiLoop.a
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        COMMENT "Removing old lib...")
    add_custom_command( TARGET ImGuiLoop POST_BUILD
                        COMMAND ar -M <ImGuiLoop.mri
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        COMMENT "Combining libs...") 
endif()

if(MSVC)
    if(NOT (MSVC_VERSION LESS 1900))
        target_link_libraries(ImGuiLoop PUBLIC legacy_stdio_definitions)
    endif()
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} ../glfw/lib/glfw3.lib ../glew/lib/glew32s.lib")
endif()


