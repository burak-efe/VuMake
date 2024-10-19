#pragma once

#include <vector>
#include "Common.h"
#include "VuGraphicsPipeline.h"
#include "VuMaterial.h"
#include "VuTexture.h"


#include "VuUtils.h"


struct VuShader {
    VkShaderModule vertexShaderModule;
    VkShaderModule fragmentShaderModule;

    VkDescriptorSetLayout imageDescriptorSetLayout;
    VuPipelineLayout vuPipelineLayout;

    VkRenderPass renderPass;

    std::vector<VuMaterial> materials;

    void dispose() {
        vkDestroyDescriptorSetLayout(Vu::Device, imageDescriptorSetLayout, nullptr);
        vkDestroyShaderModule(Vu::Device, vertexShaderModule, nullptr);
        vkDestroyShaderModule(Vu::Device, fragmentShaderModule, nullptr);
    }

    void initShader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, VkRenderPass renderPass) {
        this->renderPass = renderPass;
        materials.resize(0);

        const auto vertShaderCode = Vu::ReadFile(vertexShaderPath);
        const auto fragShaderCode = Vu::ReadFile(fragmentShaderPath);

        vertexShaderModule = Vu::CreateShaderModule(vertShaderCode);
        fragmentShaderModule = Vu::CreateShaderModule(fragShaderCode);

        createDescriptorSetLayout();
        std::array descSetLayouts{Vu::FrameConstantsDescriptorSetLayout, imageDescriptorSetLayout};
        vuPipelineLayout.CreatePipelineLayout(descSetLayouts, 64);

    }

    uint32 createMaterial(VuTexture* texture) {
        VuMaterial material;
        material.Init(
            imageDescriptorSetLayout,
            vuPipelineLayout,
            vertexShaderModule,
            fragmentShaderModule,
            renderPass,
            texture
        );
        materials.push_back(material);
        return materials.capacity() - 1;
    }

private:
    void createDescriptorSetLayout() {

        VkDescriptorSetLayoutBinding samplerLayoutBinding{
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = nullptr,
        };
        VkDescriptorSetLayoutCreateInfo imageLayout{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = 1,
            .pBindings = &samplerLayoutBinding,
        };
        VK_CHECK(vkCreateDescriptorSetLayout(Vu::Device, &imageLayout, nullptr, &imageDescriptorSetLayout));
    }
};
