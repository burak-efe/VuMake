#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>


class Shader {
public:

   static VkShaderModule CreateShaderModule(const std::vector<char> &code) {

      VkShaderModuleCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      createInfo.codeSize = code.size();
      createInfo.pCode = reinterpret_cast<const uint32 *>(code.data());
      createInfo.pNext = nullptr;

      VkShaderModule shaderModule;
      //VK_CHECK(vkCreateShaderModule(EngineContext::Device, &createInfo, nullptr, &shaderModule));
      if (vkCreateShaderModule(EngineContext::Device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
         throw std::runtime_error("failed to create shader module!");
      }

      return shaderModule;
   }

};

