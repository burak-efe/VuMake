cmake_minimum_required(VERSION 3.28)

include(FetchContent)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

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

####################################################################################################
function(add_sources_recursively target_name source_dir)
    if (NOT TARGET ${target_name})
        message(FATAL_ERROR "Target ${target_name} does not exist. Please create the target before calling this function.")
    endif ()
    file(GLOB_RECURSE sources "${source_dir}/*.cpp" "${source_dir}/*.c" "${source_dir}/*.h" "${source_dir}/*.hpp")
    if (sources)
        target_sources(${target_name} PRIVATE ${sources})
    else ()
        message(WARNING "No source files found in directory: ${source_dir}")
    endif ()
endfunction()
####################################################################################################
#Project
project("VuMake")
add_executable(${PROJECT_NAME})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
####################################################################################################
# Precompiled Headers
set(PCH
        src/common/pch.h
)
target_precompile_headers(${PROJECT_NAME} PRIVATE
        <chrono>
        <iostream>
        <print>
        <string>
        <format>
        <cstdint>
        <cassert>
        <cstdlib>
        <array>
        <vector>
        <stack>
        <algorithm>
        <functional>
        <filesystem>
        <mutex>
        <span>
        <ranges>
        ${PCH}
)
####################################################################################################
#Add Sources
add_sources_recursively(${PROJECT_NAME} src)
target_include_directories(${PROJECT_NAME} PRIVATE src/common)
target_include_directories(${PROJECT_NAME} PRIVATE src/components)
target_include_directories(${PROJECT_NAME} PRIVATE src/systems)
target_include_directories(${PROJECT_NAME} PRIVATE src/render)
target_include_directories(${PROJECT_NAME} PRIVATE src/material)
####################################################################################################
#imgui
add_sources_recursively(${PROJECT_NAME} external/imgui)
target_include_directories(${PROJECT_NAME} PRIVATE external/imgui)
####################################################################################################
#Header Onlys
target_include_directories(${PROJECT_NAME} PRIVATE external/stb)
####################################################################################################
find_package(Vulkan 1.3.296.0 REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})
####################################################################################################
FetchContent_Declare(
        fetch_volk
        GIT_REPOSITORY https://github.com/zeux/volk
        GIT_TAG vulkan-sdk-1.3.296.0
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
        GIT_TAG preview-3.1.3
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
set(GLM_BUILD_LIBRARY OFF)
FetchContent_Declare(
        fetch_glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG 1.0.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fetch_glm)
target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)
####################################################################################################
FetchContent_Declare(
        fetch_flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs
        GIT_TAG v4.0.2
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fetch_flecs)
target_link_libraries(${PROJECT_NAME} PRIVATE flecs::flecs_static)
####################################################################################################
FetchContent_Declare(
        fetch_buddy_alloc
        GIT_REPOSITORY https://github.com/spaskalev/buddy_alloc
        GIT_TAG v1.2.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fetch_buddy_alloc)
target_include_directories(${PROJECT_NAME} PRIVATE ${buddy_alloc_SOURCE_DIR}/)
####################################################################################################
#FetchContent_Declare(
#        fetch_daw_json
#        GIT_REPOSITORY https://github.com/beached/daw_json_link
#        GIT_TAG release
#        GIT_SHALLOW TRUE
#        GIT_PROGRESS TRUE
#)
#FetchContent_MakeAvailable(fetch_daw_json)
#target_link_libraries(${PROJECT_NAME} PRIVATE daw::daw-json-link)
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
#set(BUILD_SHARED_LIBS OFF)
#FetchContent_Declare(
#        fetch_asyncplusplus
#        GIT_REPOSITORY https://github.com/Amanieu/asyncplusplus
#        GIT_TAG v1.2
#        GIT_SHALLOW TRUE
#        GIT_PROGRESS TRUE
#)
#FetchContent_MakeAvailable(fetch_asyncplusplus)
#target_link_libraries(${PROJECT_NAME} PRIVATE Async++)
####################################################################################################
#set(SLANG_EMBED_CORE_MODULE OFF)
#set(SLANG_LIB_TYPE SHARED CACHE STRING FORCE)


set(SLANG_ENABLE_DXIL OFF CACHE BOOL "")
set(SLANG_ENABLE_GFX OFF CACHE BOOL "")
set(SLANG_ENABLE_SLANGD OFF CACHE BOOL "")
set(SLANG_ENABLE_SLANGC OFF CACHE BOOL "")
set(SLANG_ENABLE_SLANGRT ON CACHE BOOL "")
set(SLANG_ENABLE_SLANG_GLSLANG OFF CACHE BOOL "")
set(SLANG_ENABLE_EXAMPLES OFF CACHE BOOL "")
set(SLANG_ENABLE_SLANG_RHI OFF CACHE BOOL "")
set(SLANG_ENABLE_CUDA OFF CACHE BOOL "")
set(SLANG_ENABLE_OPTIX OFF CACHE BOOL "")
set(SLANG_ENABLE_NVAPI OFF CACHE BOOL "")
set(SLANG_ENABLE_XLIB OFF CACHE BOOL "")
set(SLANG_ENABLE_AFTERMATH OFF CACHE BOOL "")
set(SLANG_ENABLE_DX_ON_VK OFF CACHE BOOL "")
set(SLANG_ENABLE_TESTS OFF CACHE BOOL "")
set(SLANG_ENABLE_REPLAYER OFF CACHE BOOL "")
set(SLANG_ENABLE_PREBUILT_BINARIES OFF CACHE BOOL "")


FetchContent_Declare(
        fetch_slang
        GIT_REPOSITORY https://github.com/shader-slang/slang
        GIT_TAG v2024.17
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fetch_slang)
target_link_libraries(${PROJECT_NAME} PRIVATE slang)

set_target_properties(slang
        PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        $<TARGET_FILE_DIR:${PROJECT_NAME}>/Debug/bin/slang.dll $<TARGET_FILE_DIR:${PROJECT_NAME}>)
####################################################################################################
#FetchContent_Declare(
#        fetch_spirv_reflect
#        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect
#        GIT_TAG vulkan-sdk-1.3.296)
#FetchContent_MakeAvailable(fetch_spirv_reflect)
#target_sources(${PROJECT_NAME} PUBLIC ${fetch_spirv_reflect_SOURCE_DIR}/spirv_reflect.h)
#target_sources(${PROJECT_NAME} PUBLIC ${fetch_spirv_reflect_SOURCE_DIR}/spirv_reflect.cpp)
#target_include_directories(${PROJECT_NAME} PUBLIC ${fetch_spirv_reflect_SOURCE_DIR})
####################################################################################################
#FetchContent_Declare(
#        fetch_vk_bootstrap
#        GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
#        GIT_TAG v1.3.296)
#FetchContent_MakeAvailable(fetch_vk_bootstrap)
#target_link_libraries(${PROJECT_NAME} PUBLIC vk-bootstrap::vk-bootstrap)
####################################################################################################
#target_compile_definitions(${PROJECT_NAME} PUBLIC VK_NO_PROTOTYPES)
#target_compile_definitions(${PROJECT_NAME} PUBLIC VOLK_IMPLEMENTATION)
#Copy asset files to build directory
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_directory
#        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)
#if(WIN32)
#    add_custom_command(TARGET {PROJECT_NAME} POST_BUILD
#            COMMAND ${CMAKE_COMMAND} cmd ${CMAKE_SOURCE_DIR}/shaders/compile.bat)
#endif()
####################################################################################################
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/bin $<TARGET_FILE_DIR:${PROJECT_NAME}>/bin)
