#define VK_NO_PROTOTYPES
#define VOLK_IMPLEMENTATION

#include "volk.h"


#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 0
#include "vk_mem_alloc.h"


#define IMGUI_IMPL_VULKAN_USE_VOLK
#define IMGUI_IMPL_VULKAN_NO_PROTOTYPES
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"


#include "Common.h"

#define STB_IMAGE_IMPLEMENTATION

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLFW_INCLUDE_VULKAN



#include "Vu.h"
#include <stb_image.h>

#include <filesystem>

#include "flecs.h"
#include "glm/gtx/string_cast.hpp"

#include "Camera.h"
#include "Components.h"
#include "Mesh.h"
#include "Systems.h"
#include "Transform.h"
#include "VuRenderer.h"
//#include "VuTexture.h"
#include "spirv_reflect.h"


void RunEngine() {

    VuRenderer vuRenderer;
    vuRenderer.Init();
    Vu::Renderer = &vuRenderer;
    Mesh monke("shaders/monka.gltf", Vu::VmaAllocator);

    //auto texture = VuTexture("shaders/uvChecker.png");
    //texture.Dispose();

    flecs::world world;
    world.set<VuRenderer>(vuRenderer);

    //Add Systems
    auto spinningSystem = AddSpinningSystem(world);
    auto flyCameraSystem = AddFlyCameraSystem(world);
    auto renderingSystem = AddRenderingSystem(world);
    auto spinUI = AddSpinUISystem(world);
    auto trsUI = AddTransformUISystem(world);
    auto camUI = AddCameraUISystem(world);


    //Add Entities
    world.entity("Monke2").set(Transform{
        .Position = float3(0, 0, 0)
    }).set(MeshRenderer{&monke}).set(Spinn{});

    world.entity("Cam").set(
        Transform(float3(0, 0, 4.0f), float3(0, 0, 0), float3(1, 1, 1))
    ).set(Camera{});


    //Update Loop
    while (!vuRenderer.ShouldWindowClose()) {
        Vu::UpdateDeltaTime();

        //Pre-Render Begins
        spinningSystem.run();
        flyCameraSystem.run();

        //Rendering
        {

            vuRenderer.BeginFrame();
            renderingSystem.run();

            //UI
            {
                vuRenderer.BeginImgui();
                ImGui::Text(
                    std::format("Frame Per Second: {0:.0f}", (1.0f / Vu::DeltaTime))
                    .c_str());
                ImGui::Text(std::format("Frame Time as miliSec: {0:.4}", Vu::DeltaTime * 1000).c_str());
                spinUI.run();
                trsUI.run();
                camUI.run();
                vuRenderer.EndImgui();
            }

            vuRenderer.EndFrame();
        }
    }

    //Mission complete
    vuRenderer.WaitIdle();
    monke.Dispose();
    vuRenderer.Dispose();
    system("pause");
}


int main() {
    RunEngine();

    return EXIT_SUCCESS;
}
