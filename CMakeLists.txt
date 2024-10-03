cmake_minimum_required(VERSION 3.28)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif ()


function(add_sources_recursively target_name source_dir)
    # Check if the target already exists
    if(NOT TARGET ${target_name})
        message(FATAL_ERROR "Target ${target_name} does not exist. Please create the target before calling this function.")
    endif()

    # Recursively find all source files in the specified directory
    file(GLOB_RECURSE sources "${source_dir}/*.cpp" "${source_dir}/*.c" "${source_dir}/*.h" "${source_dir}/*.hpp")

    # Check if any source files were found
    if(sources)
        # Add the found sources to the specified target
        target_sources(${target_name} PRIVATE ${sources})
    else()
        message(WARNING "No source files found in directory: ${source_dir}")
    endif()
endfunction()


add_compile_options(/Bt)
#add_compile_options(/Yc)
##add_compile_options(/Yu)
#add_compile_options(/std:c++latest)
#add_compile_options(/experimental:module)
#add_compile_options(/Od)
#add_compile_options(/EHsc)
#add_compile_options(/MD)
#add_compile_options(/d2cgsummary)


project("VuMake")

#set(PCH
#        src/Common.h
#)

add_executable(${PROJECT_NAME})
add_sources_recursively(${PROJECT_NAME} src)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

#target_precompile_headers(${PROJECT_NAME} PRIVATE ${PCH})

target_include_directories(${PROJECT_NAME} PUBLIC src/common)
target_include_directories(${PROJECT_NAME} PUBLIC src/components)
target_include_directories(${PROJECT_NAME} PUBLIC src/systems)
target_include_directories(${PROJECT_NAME} PUBLIC src/render)
target_include_directories(${PROJECT_NAME} PUBLIC external/imgui)


add_sources_recursively(${PROJECT_NAME} external/imgui)


find_package(Vulkan REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Vulkan_LIBRARIES})


add_subdirectory(external/fastgltf)
target_link_libraries(${PROJECT_NAME} PRIVATE fastgltf)


add_subdirectory(external/glfw)
target_link_directories(${PROJECT_NAME} PRIVATE external/glfw/src)
target_include_directories(${PROJECT_NAME} PUBLIC external/glfw/include)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)


add_subdirectory(external/glm)
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm)


add_subdirectory(external/VulkanMemoryAllocator)
target_include_directories(${PROJECT_NAME} SYSTEM INTERFACE external/VulkanMemoryAllocator/include)
target_link_libraries(${PROJECT_NAME} PUBLIC VulkanMemoryAllocator)


add_subdirectory(external/flecs)
target_link_libraries(${PROJECT_NAME} PUBLIC flecs::flecs_static)


add_subdirectory(external/vk-bootstrap)
target_link_libraries(${PROJECT_NAME} PUBLIC vk-bootstrap::vk-bootstrap)


target_include_directories(${PROJECT_NAME} PUBLIC external/stb)

#Copy asset files to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)