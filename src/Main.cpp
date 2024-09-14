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

struct Transform {
    glm::vec3 Pos;
};

struct Spinning {
    float Rotation;
};


void RunEngine() {

    VuRenderer renderer;
    renderer.Init();

    Mesh monke("shaders/monka500k.glb", VuContext::VmaAllocator);

    while (!renderer.ShouldWindowClose()) {

        renderer.BeginFrame();
        renderer.UpdateUniformBuffer();
        renderer.RenderMesh(monke);
        //renderer.RenderImgui();
        renderer.EndFrame();
    }

    renderer.WaitIdle();
    monke.Dispose();
    renderer.Cleanup();
    system("pause");
}


int main() {
    RunEngine();
    return 0;
    flecs::world world;
    auto ent = world.entity("Monke")
            .set(Transform{.Pos = glm::vec3(0.5f, 1, 0)});

    // System declaration
    flecs::system sys = world.system<Transform>("Move")
            .each([](Transform& p) {
                // Each is invoked for each entity
                std::cout << (glm::to_string(p.Pos));
            });

    sys.run();

    return EXIT_SUCCESS;
}
