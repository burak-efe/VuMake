#pragma once
#include <vector>
#include "Common.h"
#include "VuMaterial.h"
#include "VuUtils.h"

namespace Vu {

    struct VuShaderCreateInfo {
        std::string vertexShaderPath;
        std::string fragmentShaderPath;
        VkRenderPass& renderPass;
    };

    struct VuShader {
        VkShaderModule vertexShaderModule;
        VkShaderModule fragmentShaderModule;
        VkRenderPass renderPass;
        std::vector<VuMaterial> materials;


        void init(const  VuShaderCreateInfo& createInfo) {
            ZoneScoped;
            this->renderPass = createInfo.renderPass;
            materials.resize(0);

            const auto vertShaderCode = Vu::ReadFile(createInfo.vertexShaderPath);
            const auto fragShaderCode = Vu::ReadFile(createInfo.fragmentShaderPath);

            vertexShaderModule = createShaderModule(vertShaderCode);
            fragmentShaderModule = createShaderModule(fragShaderCode);
        }

        void uninit() {
            vkDestroyShaderModule(ctx::device, vertexShaderModule, nullptr);
            vkDestroyShaderModule(ctx::device, fragmentShaderModule, nullptr);

            for (auto& material: materials) {
                material.uninit();
            }
        }

        //returns material Index
        uint32 creatematerial() {
            VuMaterial material;
            material.init({vertexShaderModule, fragmentShaderModule, renderPass});
            materials.push_back(material);
            return materials.capacity() - 1;
        }


        static VkShaderModule createShaderModule(const std::vector<char>& code) {

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
