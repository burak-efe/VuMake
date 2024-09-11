#define GLFW_INCLUDE_VULKAN
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define CGLTF_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#include <print>
#include "VuRenderer.h"
#include "Mesh.h"


int main() {
    VuRenderer renderer;
    renderer.Init();

    Mesh mesh("shaders/monka.gltf", VuContext::VmaAllocator);
    renderer.meshes.push_back(&mesh);


    while (!renderer.ShouldWindowClose()) {
        renderer.Tick();
    }
    renderer.WaitIdle();

    mesh.Dispose();


    renderer.Cleanup();
    system("pause");
    return EXIT_SUCCESS;
}
