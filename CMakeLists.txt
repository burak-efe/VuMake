cmake_minimum_required(VERSION 3.28)
include(FetchContent)
include(${CMAKE_SOURCE_DIR}/build_utils/Functions.cmake)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(VuProfileBuild "Profile Clang Build using ClangBuildAnalyzer" OFF)
option(VuRunIWYU "Run Include-What-You-use on build" OFF)
option(VuRunSanitizers "Run Address Sanitizer" OFF)



####################################################################################################
project("VuMake")
add_executable(${PROJECT_NAME} Main.cpp)
add_library(VuLibs INTERFACE)
target_link_libraries(${PROJECT_NAME} PRIVATE VuLibs)
####################################################################################################
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
# Apply compiler warnings and treat them as errors
target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra >
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra >
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)
####################################################################################################
if (VuRunSanitizers)
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
    target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
endif ()
####################################################################################################
if (VuRunIWYU)
    message("Running IWYU")
    find_program(iwyu_path NAMES include-what-you-use PATHS "${CMAKE_SOURCE_DIR}/build_utils" REQUIRED)
    set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
endif ()
####################################################################################################
target_precompile_headers(${PROJECT_NAME} PRIVATE
        "src/pch.h"
        #        <chrono> <iostream> <filesystem> <format> <algorithm>
        #        <functional> <mutex> <ranges> <array> <span> <vector> <stack> <string>
)
####################################################################################################
target_sources(VuLibs INTERFACE assets/shaders/common/InteroptStructs.h)
target_include_directories(VuLibs INTERFACE assets/shaders/common)
add_sources_recursively(VuLibs src/01_InnerCore)
add_sources_recursively(VuLibs src/02_OuterCore)
add_sources_recursively(VuLibs src/03_Mantle)
add_sources_recursively(VuLibs src/04_Crust)
add_sources_recursively(VuLibs src/10_ECS-ECF)
add_sources_recursively(VuLibs src/11_Components)
add_sources_recursively(VuLibs src/12_Systems)
add_sources_recursively(VuLibs src/13_Scenes)
add_sources_recursively(VuLibs external)
target_include_directories(VuLibs INTERFACE src)
target_include_directories(VuLibs INTERFACE external/imgui)
target_include_directories(VuLibs INTERFACE external/header_onlys)
####################################################################################################
find_package(Vulkan 1.4.309 REQUIRED)
target_include_directories(VuLibs INTERFACE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(VuLibs INTERFACE ${Vulkan_LIBRARIES})

target_compile_definitions(VuLibs INTERFACE
        #VULKAN_HPP_DISPATCH_LOADER_DYNAMIC
        VULKAN_HPP_RAII_NO_EXCEPTIONS
        VULKAN_HPP_NO_EXCEPTIONS
        VULKAN_HPP_NO_SMART_HANDLE
        VULKAN_HPP_NO_CONSTRUCTORS
)
target_compile_definitions(VuLibs INTERFACE
        VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1
)

#FetchContent_Declare(
#        fetch_vk_headers
#        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers
#        GIT_TAG v1.4.323
#        GIT_SHALLOW TRUE
#        GIT_PROGRESS TRUE
#)
#FetchContent_MakeAvailable(fetch_vk_headers)
#target_link_libraries(VuLibs INTERFACE Vulkan::Headers)

####################################################################################################
#FetchContent_Declare(
#        fetch_vkhpp
#        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Hpp
#        GIT_TAG main
#        GIT_SHALLOW TRUE
#        GIT_PROGRESS TRUE
#)
#FetchContent_MakeAvailable(fetch_vkhpp)
#target_include_directories(VuLibs INTERFACE ${fetch_vkhpp_SOURCE_DIR}/Vulkan-Headers/include)
#
#

#target_link_libraries(VuLibs INTERFACE VulkanHpp)
####################################################################################################
set(FASTGLTF_COMPILE_AS_CPP20 ON)
FetchContent_Declare(
        fetch_fastgltf
        GIT_REPOSITORY https://github.com/spnda/fastgltf
        GIT_TAG v0.8.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fetch_fastgltf)
target_link_libraries(VuLibs INTERFACE fastgltf)
####################################################################################################
set(SDL_SHARED FALSE BOOL)
set(SDL_STATIC TRUE BOOL)
set(SDL_JOYSTICK OFF)
set(SDL_HAPTIC OFF)
set(SDL_CAMERA OFF)
FetchContent_Declare(
        fetch_SDL
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        GIT_TAG release-3.2.20
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fetch_SDL)
set_property(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/_deps/fetch_sdl-src" PROPERTY EXCLUDE_FROM_ALL TRUE)
target_link_libraries(VuLibs INTERFACE SDL3::SDL3-static)
####################################################################################################
#FetchContent_Declare(
#        fetch_tracy
#        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
#        GIT_TAG v0.11.1
#        GIT_SHALLOW TRUE
#        GIT_PROGRESS TRUE
#)
#FetchContent_MakeAvailable(fetch_tracy)
#option(TRACY_ENABLE "" ON)
#option(TRACY_STATIC "" OFF)
#target_link_libraries(VuLibs INTERFACE TracyClient)
####################################################################################################
#FetchContent_Declare(
#        fetch_VMA
#        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
#        GIT_TAG v3.1.0
#        GIT_SHALLOW TRUE
#        GIT_PROGRESS TRUE
#)
#FetchContent_MakeAvailable(fetch_VMA)
#target_link_libraries(VuLibs INTERFACE VulkanMemoryAllocator)
#target_include_directories(VuLibs INTERFACE GPUOpen::VulkanMemoryAllocator)
####################################################################################################
if (VuProfileBuild)
    #Pre-Build:
    set(CBA_DIR "${CMAKE_SOURCE_DIR}/build_utils/ClangBuildAnalyzer")

    target_compile_options(${PROJECT_NAME} PRIVATE -ftime-trace)

    add_custom_target(clang_build_analyzer_start
            COMMAND ${CBA_DIR} --start ${CMAKE_BINARY_DIR}
            COMMENT "Starting ClangBuildAnalyzer"
    )

    add_dependencies(${PROJECT_NAME} clang_build_analyzer_start)

    #Post-Build:
    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CBA_DIR} --stop ${CMAKE_BINARY_DIR} build_log
            COMMENT "Stopping ClangBuildAnalyzer"
    )

    add_custom_command(TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CBA_DIR} --analyze build_log
            COMMENT "Output of  ClangBuildAnalyzer:"
    )
endif ()

#Create a symlink to the assets folder in the build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)

##Create a symlink to the bin folder in the build directory
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E create_symlink
#        ${CMAKE_SOURCE_DIR}/bin $<TARGET_FILE_DIR:${PROJECT_NAME}>/bin)

#Alert if forbidden text used
search_text_files("${CMAKE_SOURCE_DIR}/src/01_InnerCore" "02_OuterCore|03_Mantle|04_Crust")
search_text_files("${CMAKE_SOURCE_DIR}/src/02_OuterCore" "03_Mantle|04_Crust")
search_text_files("${CMAKE_SOURCE_DIR}/src/03_Mantle" "04_Crust")
#make sure vulkan headers only used in VuCommon.h
search_text_files("${CMAKE_SOURCE_DIR}/src" "vulkan.hpp|vulkan_raii.hpp")

add_subdirectory(test)