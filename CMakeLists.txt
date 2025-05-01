cmake_minimum_required(VERSION 3.28)
include(FetchContent)
include(${CMAKE_SOURCE_DIR}/build_utils/Functions.cmake)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#if (MSVC)
#    add_compile_options(/Bt) #more verbose log
#    add_compile_options(/Zi)
#    add_compile_options(/DEBUG)
#endif ()

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
#add_compile_options(-static-libasan)
#add_link_options(-static-libasan)
##add_compile_options(-fsanitize=address)
#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
#set(ENV{ASAN_OPTIONS} "asan_blacklist=${CMAKE_SOURCE_DIR}/build_utils/asan_blacklist.txt")

####################################################################################################
project("VuMake")
add_executable(${PROJECT_NAME} Main.cpp)
add_library(VuLibs INTERFACE)
target_link_libraries(${PROJECT_NAME} PRIVATE VuLibs)
####################################################################################################
#target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
#target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=address)
####################################################################################################
#set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
#set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)

#find_program(iwyu_path NAMES include-what-you-use PATHS "${CMAKE_SOURCE_DIR}/build_utils" REQUIRED)
#set_property(TARGET ${PROJECT_NAME}  PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
####################################################################################################
# Precompiled Headers
set(PCH
        src/10_Core/pch.h
)
#target_precompile_headers(${PROJECT_NAME} PRIVATE
#        <cstdint> <cassert> <cstdlib>
#        <chrono> <iostream> <filesystem>
#        <format> <algorithm> <functional>
#        <mutex> <ranges> <array>
#        <span> <vector> <stack>
#        <string>
#        #${PCH}
#)
####################################################################################################
#Add Sources
add_sources_recursively(VuLibs src)
add_sources_recursively(VuLibs external)
target_include_directories(VuLibs INTERFACE  src)
target_include_directories(VuLibs INTERFACE  external)
####################################################################################################
#imgui
target_include_directories(VuLibs INTERFACE  external/imgui)
####################################################################################################
#Header Only
target_include_directories(VuLibs INTERFACE  external/stb)
####################################################################################################
#Vulkan
find_package(Vulkan 1.4.304 REQUIRED)
target_include_directories(VuLibs INTERFACE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(VuLibs INTERFACE ${Vulkan_LIBRARIES})
####################################################################################################
FetchContent_Declare(
        fetch_volk
        GIT_REPOSITORY https://github.com/zeux/volk
        GIT_TAG vulkan-sdk-1.4.304.1
)
FetchContent_MakeAvailable(fetch_volk)
if (WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif ()
target_link_libraries(VuLibs INTERFACE volk)
####################################################################################################
FetchContent_Declare(
        fetch_VMA
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        GIT_TAG v3.1.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fetch_VMA)
target_link_libraries(VuLibs INTERFACE VulkanMemoryAllocator)
target_include_directories(VuLibs INTERFACE GPUOpen::VulkanMemoryAllocator)
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
FetchContent_Declare(
        fetch_flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs
        GIT_TAG v4.0.4
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fetch_flecs)
target_link_libraries(VuLibs INTERFACE flecs::flecs_static)
####################################################################################################
FetchContent_Declare(
        fetch_tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG v0.11.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fetch_tracy)
option(TRACY_ENABLE "" ON)
option(TRACY_STATIC "" OFF)
target_link_libraries(VuLibs INTERFACE TracyClient)
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
search_text_files("${CMAKE_SOURCE_DIR}/src" "#include <vector>")

add_subdirectory(test)