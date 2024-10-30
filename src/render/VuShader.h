#pragma once

#include <vector>
#include "Common.h"
#include "VuGraphicsPipeline.h"
#include "VuMaterial.h"

#include "VuUtils.h"

namespace Vu {
    struct VuShader {
        VkShaderModule vertexShaderModule;
        VkShaderModule fragmentShaderModule;

        VkRenderPass renderPass;

        std::vector<VuMaterial> materials;

        void Dispose() {
            vkDestroyShaderModule(ctx::device, vertexShaderModule, nullptr);
            vkDestroyShaderModule(ctx::device, fragmentShaderModule, nullptr);

            for (auto& material: materials) {
                material.Dispose();
            }
        }

        void InitShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, VkRenderPass renderPass) {
            this->renderPass = renderPass;
            materials.resize(0);

            const auto vertShaderCode = Vu::ReadFile(vertexShaderPath);
            const auto fragShaderCode = Vu::ReadFile(fragmentShaderPath);

            vertexShaderModule = Vu::CreateShaderModule(vertShaderCode);
            fragmentShaderModule = Vu::CreateShaderModule(fragShaderCode);
        }

        //returns material Index
        uint32 CreateMaterial() {

            VuMaterial material;
            material.Init(vertexShaderModule, fragmentShaderModule, renderPass);
            materials.push_back(material);
            return materials.capacity() - 1;
        }
    };
}
