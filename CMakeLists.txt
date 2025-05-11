cmake_minimum_required(VERSION 3.28)
include(FetchContent)
include(${CMAKE_SOURCE_DIR}/build_utils/Functions.cmake)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


####################################################################################################
project("VuMake")
add_executable(${PROJECT_NAME} Main.cpp)
add_library(VuLibs INTERFACE)
target_link_libraries(${PROJECT_NAME} PRIVATE VuLibs)
####################################################################################################
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
####################################################################################################
#target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
#target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
####################################################################################################
find_program(iwyu_path NAMES include-what-you-use PATHS "${CMAKE_SOURCE_DIR}/build_utils" REQUIRED)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
####################################################################################################
target_precompile_headers(${PROJECT_NAME} PRIVATE
        "src/pch.h"
        #        <cstdint> <cassert> <cstdlib> <chrono> <iostream> <filesystem> <format> <algorithm>
        #        <functional> <mutex> <ranges> <array> <span> <vector> <stack> <string>
)
####################################################################################################
add_sources_recursively(VuLibs src/01_InnerCore)
add_sources_recursively(VuLibs src/02_OuterCore)
add_sources_recursively(VuLibs src/03_Mantle)
add_sources_recursively(VuLibs external)
target_include_directories(VuLibs INTERFACE src)
target_include_directories(VuLibs INTERFACE external/imgui)
target_include_directories(VuLibs INTERFACE external/header_onlys)
####################################################################################################
find_package(Vulkan 1.4.309 REQUIRED)
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
#target_compile_definitions(VuLibs INTERFACE VULKAN_HPP_EXPECTED=tl::expected)
target_include_directories(VuLibs INTERFACE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(VuLibs INTERFACE ${Vulkan_LIBRARIES})

#add_library(VulkanHppModule)
#
#target_sources(VulkanHppModule PUBLIC
#        FILE_SET CXX_MODULES
#        BASE_DIRS ${Vulkan_INCLUDE_DIR}
#        FILES ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
#)
#target_compile_features(VulkanHppModule PUBLIC cxx_std_20)
#target_compile_definitions(VulkanHppModule PUBLIC
#        VULKAN_HPP_RAII_NO_EXCEPTIONS
#        VULKAN_HPP_NO_EXCEPTIONS
#        VULKAN_HPP_NO_SMART_HANDLE
#        #VULKAN_HPP_NO_CONSTRUCTORS
#)
#target_link_libraries(VulkanHppModule PUBLIC Vulkan::Vulkan)
#target_link_libraries(VuLibs INTERFACE VulkanHppModule)
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
        GIT_TAG release-3.2.4
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
#Create a symlink to the assets folder in the build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)

#Create a symlink to the bin folder in the build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/bin $<TARGET_FILE_DIR:${PROJECT_NAME}>/bin)

#Alert if forbidden text used
#search_text_files("${CMAKE_SOURCE_DIR}/src" "std::vector|std::stack|vk_mem")
add_subdirectory(test)