cmake_minimum_required(VERSION 3.28)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif ()

project("VuMake")

add_executable(${PROJECT_NAME}
        src/Main.cpp
        src/VuRenderer.h
        src/VuRenderer.cpp
        src/Mesh.h
        src/Mesh.cpp
        src/Common.h
        src/VuBuffer.h
        src/VuBuffer.cpp
        src/VuTypes.h
        src/VuUtils.h
        src/VuSwapChain.cpp
        src/VuSwapChain.h
        src/VuInit.h
        src/VuShader.h
        src/VuContext.h
        src/VuGraphicsPipeline.h
        src/VuDeviceUtils.h
        src/VuDepthStencil.h

        src/imgui/imconfig.h
        src/imgui/imgui.cpp
        src/imgui/imgui.h
        src/imgui/imgui_demo.cpp
        src/imgui/imgui_draw.cpp
        src/imgui/imgui_tables.cpp
        src/imgui/imgui_widgets.cpp
        src/imgui/imgui_internal.h
        src/imgui/imgui_impl_vulkan.cpp
        src/imgui/imgui_impl_glfw.h
        src/imgui/imgui_impl_glfw.cpp
        src/imgui/imgui_impl_vulkan.h
        src/imgui/imstb_rectpack.h
        src/imgui/imstb_textedit.h
        src/imgui/imstb_truetype.h
        src/imgui/imgui_impl_glfw.cpp
        src/LegitProfiler/ImGuiProfilerRenderer.h
        src/LegitProfiler/ProfilerTask.h

)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

#include(FetchContent)
#
#FetchContent_Declare(imgui
#        URL https://github.com/ocornut/imgui/archive/refs/tags/v1.91.0.tar.gz
#        EXCLUDE_FROM_ALL
#)
#FetchContent_MakeAvailable(imgui)
#
#target_sources(${PROJECT_NAME} PRIVATE
#        ${imgui_SOURCE_DIR}/imgui.h
#        ${imgui_SOURCE_DIR}/imgui.cpp
#        ${imgui_SOURCE_DIR}/imconfig.h
#        ${imgui_SOURCE_DIR}/imgui_internal.h
#        ${imgui_SOURCE_DIR}/imgui_demo.cpp
#        ${imgui_SOURCE_DIR}/imgui_draw.cpp
#        ${imgui_SOURCE_DIR}/imgui_tables.cpp
#        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
#        ${imgui_SOURCE_DIR}/backends/imgui_impl_allegro5.h
#        #${imgui_SOURCE_DIR}/backends/imgui_impl_allegro5.cpp
#        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.h
#        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
#        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h
#        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
#)
#target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR})
#target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR/backends})
#target_link_libraries(${PROJECT_NAME} PRIVATE imgui)


#set(VULKAN_SDK "C:/VulkanSDK/1.3.290.0")
#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${VULKAN_SDK}/lib/cmake")

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



#Copy asset files to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)