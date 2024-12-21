#pragma once

#include <filesystem>

#include "flecs.h"
#include "glm/gtx/string_cast.hpp"
#include "tracy/Tracy.hpp"
//#include "async++.h"
#include "imgui_internal.h"

#include "Camera.h"
#include "Components.h"
#include "VuMesh.h"
#include "Systems.h"
#include "Transform.h"
#include "VuAssetLoader.h"
#include "VuResourceManager.h"
#include "VuRenderer.h"


namespace Vu {

    struct Scene0 {
    private:
        std::filesystem::path helmetPath = "D:/Dev/Vulkan/glTF-Sample-Assets/Models/DamagedHelmet/glTF/DamagedHelmet.gltf";
        std::filesystem::path gnomePath = "assets/gltf/garden_gnome/garden_gnome_2k.gltf";

        VuHandle<VuShader> shader;

        flecs::system spinningSystem;
        flecs::system flyCameraSystem;
        flecs::system drawMeshSystem;
        flecs::system spinUI;
        flecs::system trsUI;
        flecs::system camUI;

        VuRenderer vuRenderer;

        void UpdateLoop() {
            ZoneScoped;
            //Update Loop
            while (!vuRenderer.shouldWindowClose()) {
                ctx::PreUpdate();
                ctx::UpdateInput();

                //Pre-Render Begins
                auto runner0 = spinningSystem.run();
                auto runner1 = flyCameraSystem.run();

                //Rendering
                {
                    shader.get().tryRecompile();

                    vuRenderer.beginFrame();
                    drawMeshSystem.run();

                    //UI
                    {
                        vuRenderer.beginImgui();

                        uint32 dockspace_flags = 0;
                        ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
                        ImGui::DockSpaceOverViewport(dockspace_id, nullptr, ImGuiDockNodeFlags_PassthruCentralNode, nullptr);

                        auto wRes = ImGui::Begin("Info");
                        ImGui::Text("Texture Count: %u", VuPool<VuTexture>::getUsedSlotCount());
                        ImGui::Text("Sampler Count: %u", VuPool<VuSampler>::getUsedSlotCount());
                        ImGui::Text("Buffer Count: %u", VuPool<VuBuffer>::getUsedSlotCount());
                        ImGui::End();

                        static auto first_time = true;
                        if (first_time) {
                            first_time = false;

                            ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
                            ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
                            ImGui::DockBuilderSetNodeSize(dockspace_id, {
                                                              static_cast<float>(vuRenderer.swapChain.swapChainExtent.width),
                                                              static_cast<float>(vuRenderer.swapChain.swapChainExtent.height)
                                                          });

                            auto mainR = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_id);
                            auto mainL = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left, 0.33f, nullptr, &dockspace_id);
                            ImGui::DockBuilderDockWindow("Info", mainL);
                            // ImGui::DockBuilderDockWindow("Meshes", mainL);
                            // ImGui::DockBuilderDockWindow("Shaders", mainL);

                            ImGui::DockBuilderFinish(dockspace_id);
                        }


                        vuRenderer.endImgui();
                    }

                    vuRenderer.endFrame();
                }
            }
        }

    public:
        void Run() {
            auto device = VuDevice{};
            ctx::vuDevice = &device;
            ZoneScoped;
            std::cout << "Scene init" << std::endl;
            vuRenderer.init();
            ctx::vuRenderer = &vuRenderer;

            // VuHandle<VuBuffer> testSSBO;
            // testSSBO.createHandle().init({
            //     .lenght = 3, .strideInBytes = 4,
            //     .vkUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            //     .vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
            //     .vmaCreateFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
            // });
            //float up[] = {0,4,0};
            //testSSBO.get().setData(up, sizeof(up), 0);
            ///VuResourceManager::registerStorageBuffer(testSSBO.index,testSSBO.get());




            VuMesh mesh{};

            shader.createHandle().initAsGraphicsShader(
                {
                    "assets/shaders/vert.slang",
                    "assets/shaders/frag.slang",
                    vuRenderer.swapChain.renderPass.renderPass
                }
            );

            uint32 mat0 = shader.get().createMaterial();

            GPU_PBR_MaterialData* data = shader.get().materials[mat0].pbrMaterialData;
            VuAssetLoader::LoadGltf(gnomePath, mesh, *data);


            //
            {
                ZoneScopedN("flecs");
                flecs::world world;
                //Add Systems
                spinningSystem = AddSpinningSystem(world);
                flyCameraSystem = AddFlyCameraSystem(world);
                drawMeshSystem = AddRenderingSystem(world);
                spinUI = AddSpinUISystem(world);
                trsUI = AddTransformUISystem(world);
                camUI = AddCameraUISystem(world);

                //Add Entities
                auto ent = world.entity("Obj1").set(Transform{
                    .Position = float3(0, 0, 0), .Rotation = glm::quat(glm::vec3{0, 0, 0}), .Scale = {1, 1, 1}
                }).set(MeshRenderer{&mesh, shader, mat0}).set(Spinn{});
                //shader.increaseRefCount();

                world.entity("Cam").set(
                    Transform(glm::vec3(0.0f, 0.0f, 3.5f), float3(0, 0, 0), float3(1, 1, 1))
                ).set(Camera{});

                UpdateLoop();

            }
            vuRenderer.waitIdle();
            //VuResourceManager::decreaseTextureRefCount(shader.materials[mat0].pbrMaterialData->texture0);
            //VuResourceManager::decreaseTextureRefCount(shader.materials[mat0].pbrMaterialData->texture1);
            shader.destroyHandle();
            mesh.uninit();
            vuRenderer.uninit();
        }
    };
}
