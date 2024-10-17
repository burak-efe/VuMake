#pragma once

#include <vector>
#include "Common.h"

#include "VuUtils.h"


class VuShader {
public:

   static VkShaderModule CreateShaderModule(const std::vector<char> &code) {

      VkShaderModuleCreateInfo createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      createInfo.codeSize = code.size();
      createInfo.pCode = reinterpret_cast<const uint32 *>(code.data());
      createInfo.pNext = nullptr;

      VkShaderModule shaderModule;
      VK_CHECK(vkCreateShaderModule(Vu::Device, &createInfo, nullptr, &shaderModule));
      return shaderModule;
   }

};

