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
endif ()

function(add_sources_recursively target_name source_dir)
    if (NOT TARGET ${target_name})
        message(FATAL_ERROR "Target ${target_name} does not exist. Please create the target before calling this function.")
    endif ()
    file(GLOB_RECURSE sources "${source_dir}/*.cpp" "${source_dir}/*.c" "${source_dir}/*.h" "${source_dir}/*.hpp")
    if (sources)
        #message(${sources})
        target_sources(${target_name} PUBLIC ${sources})
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
#target_precompile_headers(${PROJECT_NAME} PUBLIC ${PCH})
target_precompile_headers(${PROJECT_NAME} PUBLIC
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
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})
####################################################################################################
#FetchContents
FetchContent_Declare(
        fetch_volk
        GIT_REPOSITORY https://github.com/zeux/volk
        GIT_TAG vulkan-sdk-1.3.296.0
)
FetchContent_MakeAvailable(fetch_volk)

if (WIN32)
    set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)
endif ()
target_link_libraries(${PROJECT_NAME} PUBLIC volk)
####################################################################################################
FetchContent_Declare(
        fetch_VMA
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        GIT_TAG v3.1.0)
FetchContent_MakeAvailable(fetch_VMA)
target_link_libraries(${PROJECT_NAME} PUBLIC VulkanMemoryAllocator)
target_include_directories(${PROJECT_NAME} PUBLIC GPUOpen::VulkanMemoryAllocator)
####################################################################################################
FetchContent_Declare(
        fetch_spirv_reflect
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect
        GIT_TAG vulkan-sdk-1.3.296)
FetchContent_MakeAvailable(fetch_spirv_reflect)
target_sources(${PROJECT_NAME} PUBLIC ${fetch_spirv_reflect_SOURCE_DIR}/spirv_reflect.h)
target_sources(${PROJECT_NAME} PUBLIC ${fetch_spirv_reflect_SOURCE_DIR}/spirv_reflect.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${fetch_spirv_reflect_SOURCE_DIR})
####################################################################################################
#FetchContent_Declare(
#        fetch_vk_bootstrap
#        GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
#        GIT_TAG v1.3.296)
#FetchContent_MakeAvailable(fetch_vk_bootstrap)
#target_link_libraries(${PROJECT_NAME} PUBLIC vk-bootstrap::vk-bootstrap)
####################################################################################################
set(SDL_STATIC TRUE CACHE BOOL "Build a SDL static library (if available)")
set(SDL_JOYSTICK OFF)
set(SDL_HAPTIC OFF)
set(SDL_CAMERA OFF)
FetchContent_Declare(
        fetch_SDL
        GIT_REPOSITORY https://github.com/libsdl-org/SDL
        GIT_TAG preview-3.1.3)
FetchContent_MakeAvailable(fetch_SDL)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3)
####################################################################################################
FetchContent_Declare(
        fetch_fastgltf
        GIT_REPOSITORY https://github.com/spnda/fastgltf
        GIT_TAG v0.8.0)
FetchContent_MakeAvailable(fetch_fastgltf)
target_link_libraries(${PROJECT_NAME} PUBLIC fastgltf)
####################################################################################################
FetchContent_Declare(
        fetch_glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG 1.0.1)
FetchContent_MakeAvailable(fetch_glm)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)
####################################################################################################
FetchContent_Declare(
        fetch_flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs
        GIT_TAG v4.0.2)
FetchContent_MakeAvailable(fetch_flecs)
target_link_libraries(${PROJECT_NAME} PUBLIC flecs::flecs_static)
####################################################################################################
FetchContent_Declare(
        fetch_buddy_alloc
        GIT_REPOSITORY https://github.com/spaskalev/buddy_alloc
        GIT_TAG v1.2.0)
FetchContent_MakeAvailable(fetch_buddy_alloc)
target_include_directories(${PROJECT_NAME} PUBLIC ${buddy_alloc_SOURCE_DIR}/)
####################################################################################################
FetchContent_Declare(
        fetch_daw_json
        GIT_REPOSITORY https://github.com/beached/daw_json_link
        GIT_TAG release)
FetchContent_MakeAvailable(fetch_daw_json)
target_link_libraries(${PROJECT_NAME} PUBLIC daw::daw-json-link)
####################################################################################################
FetchContent_Declare (
        fetch_tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG v0.11.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
FetchContent_MakeAvailable(fetch_tracy)
option (TRACY_ENABLE "" ON)
option (TRACY_STATIC "" ON)
target_link_libraries(${PROJECT_NAME} PUBLIC TracyClient)
####################################################################################################
FetchContent_Declare (
        fetch_asyncplusplus
        GIT_REPOSITORY https://github.com/Amanieu/asyncplusplus
        GIT_TAG v1.2
)
FetchContent_MakeAvailable(fetch_asyncplusplus)
target_link_libraries(${PROJECT_NAME} PUBLIC Async++)
####################################################################################################
#set(SLANG_ENABLE_SLANGD OFF)
#set(SLANG_ENABLE_SLANGC OFF)
#set(SLANG_ENABLE_TESTS OFF)
#set(SLANG_ENABLE_EXAMPLES OFF)
#set(SLANG_ENABLE_SLANG_RHI OFF)

FetchContent_Declare (
        fetch_slang
        GIT_REPOSITORY https://github.com/shader-slang/slang
        GIT_TAG v2024.14.6
)
FetchContent_MakeAvailable(fetch_slang)
target_link_libraries(${PROJECT_NAME} PUBLIC slang)


# Get the linked libraries
get_target_property(linked_libs ${PROJECT_NAME} INTERFACE_LINK_LIBRARIES)

# Print the linked libraries
if(linked_libs)
    message("Libraries linked to my_library: ${linked_libs}")
else()
    message("No libraries linked to my_library.")
endif()

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

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_SOURCE_DIR}/bin $<TARGET_FILE_DIR:${PROJECT_NAME}>/bin)
