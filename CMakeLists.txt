cmake_minimum_required(VERSION 3.28)

include(FetchContent)

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    #cmake_policy(SET CMP0141 NEW)
    #set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif ()

if (MSVC)
    add_compile_options(/Bt) #more verbose log
    add_compile_options(/Zi)
    add_compile_options(/DEBUG)
endif ()


include(${CMAKE_SOURCE_DIR}/buildUtils/Functions.cmake)

####################################################################################################
#Project
project("VuMake")
add_executable(${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD_REQUIRED ON)
add_library(VuLibs STATIC src/main.cpp)
####################################################################################################
# Precompiled Headers
set(PCH
        src/10_Core/pch.h
)
target_precompile_headers(${PROJECT_NAME} PRIVATE
        <cstdint> <cassert> <cstdlib>
        <chrono> <iostream> <filesystem>
        <format> <algorithm> <functional>
        <mutex> <ranges> <array>
        <span> <vector> <stack>
        <string>
        ${PCH}
)
####################################################################################################
#Add Sources
add_sources_recursively(${PROJECT_NAME} src)
add_sources_recursively(${PROJECT_NAME} external)
target_include_directories(${PROJECT_NAME} PRIVATE src)
####################################################################################################
#imgui
target_include_directories(${PROJECT_NAME} PRIVATE external/imgui)
####################################################################################################
#Header Only
target_include_directories(${PROJECT_NAME} PRIVATE external/stb)
####################################################################################################
#Vulkan
find_package(Vulkan 1.4.304 REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})
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
target_link_libraries(${PROJECT_NAME} PRIVATE volk)
####################################################################################################
FetchContent_Declare(
        fetch_VMA
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        GIT_TAG v3.1.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fetch_VMA)
target_link_libraries(${PROJECT_NAME} PRIVATE VulkanMemoryAllocator)
target_include_directories(${PROJECT_NAME} PRIVATE GPUOpen::VulkanMemoryAllocator)
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
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3-static)
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
target_link_libraries(${PROJECT_NAME} PRIVATE fastgltf)
####################################################################################################
FetchContent_Declare(
        fetch_flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs
        GIT_TAG v4.0.4
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fetch_flecs)
target_link_libraries(${PROJECT_NAME} PRIVATE flecs::flecs_static)
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
target_link_libraries(${PROJECT_NAME} PRIVATE TracyClient)
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