#include "VuShader.h"

#include <algorithm>  // for max
#include <cassert>    // for assert
#include <filesystem> // for path
#include <format>     // for format
#include <optional>
#include <span>
#include <stdlib.h>
#include <string>  // for basic_string, string
#include <utility> // for pair
#include <vector>  // for vector

#include "../02_OuterCore/VuCommon.h"
#include "01_InnerCore/TypeDefs.h" // for u32
#include "01_InnerCore/VuLogger.h" // for Logger
#include "02_OuterCore/VuConfig.h"
#include "02_OuterCore/VuIO.h"
#include "03_Mantle/VuDevice.h"
#include "03_Mantle/VuRenderPass.h"
#include "04_Crust/VuMaterial.h"
#include "VuRenderer.h"


Vu::VuShader::VuShader(std::shared_ptr<VuRenderer>   vuRenderer,
                       std::shared_ptr<VuRenderPass> vuRenderPass,
                       path                          vertexShaderPath,
                       path                          fragmentShaderPath) :
    m_vuRenderer {vuRenderer},
    m_vuRenderPass {vuRenderPass} {
  this->m_vertexShaderPath   = vertexShaderPath;
  this->m_fragmentShaderPath = fragmentShaderPath;

  m_lastModifiedTime = std::max(getlastModifiedTime(vertexShaderPath), getlastModifiedTime(fragmentShaderPath));

  auto vertOutPath = compileToSpirv(vertexShaderPath);
  auto fragOutPath = compileToSpirv(fragmentShaderPath);

  const auto vertSpv = Vu::readFile(vertOutPath);
  const auto fragSpv = Vu::readFile(fragOutPath);

  if (vertSpv.has_value()) {
    m_vertexShaderModule = createShaderModule(*vuRenderer->m_vuDevice, vertSpv.value().data(), vertSpv.value().size());
  } else {
    Logger::Error("vertex spv file cannot be read!");
  }

  if (fragSpv.has_value()) {
    m_fragmentShaderModule = createShaderModule(*vuRenderer->m_vuDevice, fragSpv.value().data(), fragSpv.value().size());
  } else {
    Logger::Error("frag spv file cannot be read!");
  }
}

void
Vu::VuShader::tryRecompile() {
  auto maxTime = std::max(getlastModifiedTime(m_vertexShaderPath), getlastModifiedTime(m_fragmentShaderPath));
  if (maxTime <= m_lastModifiedTime) { return; }

  // store material settings to recreate them later
  std::vector<MaterialSettings> currentlyAvailableMatSettings;
  std::vector<std::string>      keys;
  keys.reserve(m_compiledPipelines.size());
  for (const auto& pair : m_compiledPipelines) {
    currentlyAvailableMatSettings.push_back(pair.first);
  }

  VkResult waitRes = vkDeviceWaitIdle(m_vuRenderer->m_vuDevice->m_device);
  THROW_if_fail(waitRes);

  auto newVuShaderOrErr = make(m_vuRenderer, m_vuRenderPass, m_vertexShaderPath, m_fragmentShaderPath);
  THROW_if_unexpected(newVuShaderOrErr);
  *this = std::move(newVuShaderOrErr.value());

  for (auto& setting : currentlyAvailableMatSettings) {

    auto pipelineOrErr = VuGraphicsPipeline::make(m_vuRenderer->m_vuDevice,
                                                  m_vuRenderer->m_globalPipelineLayout,
                                                  m_vertexShaderModule,
                                                  m_fragmentShaderModule,
                                                  m_vuRenderPass->m_renderPass,
                                                  m_vuRenderPass->m_colorBlendAttachmentStates);

    m_compiledPipelines.emplace(setting, move_or_THROW(pipelineOrErr));
  }
}

Vu::VuGraphicsPipeline&
Vu::VuShader::requestPipeline(MaterialSettings materialSettings) {
  bool contains = m_compiledPipelines.contains(materialSettings);
  if (!contains) {
    auto pipelineOrErr = VuGraphicsPipeline::make(m_vuRenderer->m_vuDevice,
                                                  m_vuRenderer->m_globalPipelineLayout,
                                                  m_vertexShaderModule,
                                                  m_fragmentShaderModule,
                                                  m_vuRenderPass->m_renderPass,
                                                  m_vuRenderPass->m_colorBlendAttachmentStates);

    m_compiledPipelines.emplace(materialSettings, move_or_THROW(pipelineOrErr));
  }
  return m_compiledPipelines[materialSettings];
}

VkShaderModule
Vu::VuShader::createShaderModule(const VuDevice& vuDevice, const void* code, size_t size) {
  VkShaderModuleCreateInfo createInfo {.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
  createInfo.codeSize = size;
  createInfo.pCode    = static_cast<const u32*>(code);
  createInfo.pNext    = nullptr;

  VkShaderModule shaderModule    = VK_NULL_HANDLE;
  VkResult       shaderModuleRes = vkCreateShaderModule(vuDevice.m_device, &createInfo, NO_ALLOC_CALLBACK, &shaderModule);

  THROW_if_fail(shaderModuleRes);
  return shaderModule;
}

path
Vu::VuShader::compileToSpirv(const path& shaderCodePath) {
  path shaderPath = shaderCodePath;
  shaderPath.make_preferred();

  path spirvFilePath = shaderCodePath;
  spirvFilePath.make_preferred();
  spirvFilePath.replace_extension(".spv");

  path compilerPath = config::getShaderCompilerPath();
  compilerPath.make_preferred();

  std::string cmd = std::format("{0} {1} -target spirv -fvk-use-scalar-layout -o {2}",
                                compilerPath.string(),
                                shaderPath.string(),
                                spirvFilePath.string());

  int compileResult = system(cmd.c_str());
  assert(compileResult == 0);
  return spirvFilePath;
}
