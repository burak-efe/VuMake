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
                material.dispose();
            }
        }

        void CreateShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, VkRenderPass renderPass) {
            this->renderPass = renderPass;
            materials.resize(0);

            const auto vertShaderCode = Vu::ReadFile(vertexShaderPath);
            const auto fragShaderCode = Vu::ReadFile(fragmentShaderPath);

            vertexShaderModule = CreateShaderModule(vertShaderCode);
            fragmentShaderModule = CreateShaderModule(fragShaderCode);
        }

        //returns material Index
        uint32 CreateMaterial() {

            VuMaterial material;
            material.init(vertexShaderModule, fragmentShaderModule, renderPass);
            materials.push_back(material);
            return materials.capacity() - 1;
        }


        static VkShaderModule CreateShaderModule(const std::vector<char>& code) {

            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const uint32 *>(code.data());
            createInfo.pNext = nullptr;

            VkShaderModule shaderModule;
            VkCheck(vkCreateShaderModule(ctx::device, &createInfo, nullptr, &shaderModule));
            return shaderModule;
        }
    };
}
