cmake_minimum_required(VERSION 3.28)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif ()


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

set(HEADER_FILES
        src/Common.h
        src/VuRenderer.h
        src/Mesh.h
        src/VuBuffer.h
        src/VuUtils.h
        src/VuSwapChain.h
        src/VuSync.h
        src/VuShader.h
        src/Vu.h
        src/VuDepthStencil.h
        src/VuGraphicsPipeline.h
        src/VuTexture.h
        src/components/Camera.h
        src/components/Transform.h
        #Imgui
        src/imgui/imgui.h
        src/imgui/imconfig.h
        src/imgui/imgui_internal.h
        src/imgui/imgui_impl_glfw.h
        src/imgui/imgui_impl_vulkan.h
        src/imgui/imstb_rectpack.h
        src/imgui/imstb_textedit.h
        src/imgui/imstb_truetype.h
)

set(SOURCE_FILES
        src/main.cpp
        src/VuRenderer.cpp
        src/Mesh.cpp
        src/VuBuffer.cpp
        src/VuSwapChain.cpp
        src/VuRenderer.Init.cpp
        src/VuTexture.cpp
        src/VuUtils.cpp

        src/imgui/imgui.cpp
        src/imgui/imgui_demo.cpp
        src/imgui/imgui_draw.cpp
        src/imgui/imgui_tables.cpp
        src/imgui/imgui_widgets.cpp
        src/imgui/imgui_impl_vulkan.cpp
        src/imgui/imgui_impl_glfw.cpp
        src/imgui/imgui_impl_glfw.cpp
        ${HEADER_FILES}
)

set(PCH
        src/Common.h
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)
target_precompile_headers(${PROJECT_NAME} PRIVATE ${PCH})

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