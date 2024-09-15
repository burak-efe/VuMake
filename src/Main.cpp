#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define CGLTF_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#define GLM_ENABLE_EXPERIMENTAL

#include "VuRenderer.h"
#include "Mesh.h"
#include "flecs.h"
#include "glm/gtx/string_cast.hpp"

struct MeshRenderer {
    Mesh* Mesh;
};

struct Transform {
    glm::mat4 TRS = glm::identity<glm::mat4>();
};

struct Spinn {
    glm::vec3 Axis = glm::vec3(0, 1, 0);
    float Angle = 1;
};


void RunEngine() {
    float deltaTime = 0;

    VuRenderer vuRenderer;
    vuRenderer.Init();
    VuContext::Renderer = &vuRenderer;
    Mesh monke("shaders/monka500k.glb", VuContext::VmaAllocator);

    flecs::world world;
    world.set<VuRenderer>(vuRenderer);

    world.entity("Monke").set(Transform{}).set(MeshRenderer{&monke}).set(Spinn{});


    auto renderingSystem = world.system<Transform, MeshRenderer>("Rendering")
            .each([](Transform& trs, MeshRenderer& meshRenderer) {
                VuContext::Renderer->RenderMesh(*meshRenderer.Mesh, trs.TRS);
            });

    auto spinningSystem = world.system<Transform, Spinn>("SpinningSystem")
            .each([](Transform& trs, Spinn& spinn) {
                trs.TRS = glm::rotate(trs.TRS, spinn.Angle, spinn.Axis);
            });


    while (!vuRenderer.ShouldWindowClose()) {
        vuRenderer.BeginFrame();
        spinningSystem.run();
        vuRenderer.UpdateUniformBuffer();
        renderingSystem.run();
        vuRenderer.RenderImgui();
        vuRenderer.EndFrame();
    }

    vuRenderer.WaitIdle();
    monke.Dispose();
    vuRenderer.Dispose();
    system("pause");
}


int main() {
    RunEngine();


    return EXIT_SUCCESS;
}
