#include "VuShader.h"

#include <algorithm>  // for max
#include <cassert>    // for assert
#include <filesystem> // for path
#include <format>     // for format
#include <string>     // for basic_string, string
#include <utility>    // for pair
#include <vector>     // for vector

#include "01_InnerCore/TypeDefs.h" // for u32
#include "01_InnerCore/VuLogger.h" // for Logger
#include "02_OuterCore/VuConfig.h"
#include "02_OuterCore/VuIO.h"
#include "03_Mantle/VuDevice.h"
#include "VuRenderer.h"

std::expected<Vu::VuShader, vk::Result>
Vu::VuShader::make(const std::shared_ptr<VuRenderer>&   vuRenderer,
                   const std::shared_ptr<VuRenderPass>& vuRenderPass,
                   const path&                          vertexShaderPath,
                   const path&                          fragmentShaderPath) {

  try {
    VuShader vuShader {vuRenderer, vuRenderPass, vertexShaderPath, fragmentShaderPath};
    return vuShader;
  } catch (vk::Result result) {
    return std::unexpected {result}; //
  } catch (...) {
    return std::unexpected {vk::Result::eErrorUnknown}; //
  }
}

Vu::VuShader::VuShader(std::shared_ptr<VuRenderer>   vuRenderer,
                       std::shared_ptr<VuRenderPass> vuRenderPass,
                       path                          vertexShaderPath,
                       path                          fragmentShaderPath)
    : vuRenderer {vuRenderer}, vuRenderPass {vuRenderPass} {
  this->vertexShaderPath   = vertexShaderPath;
  this->fragmentShaderPath = fragmentShaderPath;

  lastModifiedTime = std::max(getlastModifiedTime(vertexShaderPath), getlastModifiedTime(fragmentShaderPath));

  auto vertOutPath = compileToSpirv(vertexShaderPath);
  auto fragOutPath = compileToSpirv(fragmentShaderPath);

  const auto vertSpv = Vu::readFile(vertOutPath);
  const auto fragSpv = Vu::readFile(fragOutPath);

  if (vertSpv.has_value()) {
    vertexShaderModule = createShaderModule(*vuRenderer->vuDevice, vertSpv.value().data(), vertSpv.value().size());
  } else {
    Logger::Error("vertex spv file cannot be read!");
  }

  if (fragSpv.has_value()) {
    fragmentShaderModule = createShaderModule(*vuRenderer->vuDevice, fragSpv.value().data(), fragSpv.value().size());
  } else {
    Logger::Error("frag spv file cannot be read!");
  }
}

// void Vu::VuShader::uninit()
// {
//     vkDestroyShaderModule(vuDevice->device, vertexShaderModule, nullptr);
//     vkDestroyShaderModule(vuDevice->device, fragmentShaderModule, nullptr);
//
//     for (auto& pipeline : compiledPipelines)
//     {
//         pipeline.second.uninit();
//     }
// }

void
Vu::VuShader::tryRecompile() {
  auto maxTime = std::max(getlastModifiedTime(vertexShaderPath), getlastModifiedTime(fragmentShaderPath));
  if (maxTime <= lastModifiedTime) { return; }

  // store material settings to recreate them later
  std::vector<MaterialSettings> currentlyAvailableMatSettings;
  std::vector<std::string>      keys;
  keys.reserve(compiledPipelines.size());
  for (const auto& pair : compiledPipelines) {
    currentlyAvailableMatSettings.push_back(pair.first);
  }

  vuRenderer->vuDevice->device.waitIdle();

  auto newVuShaderOrErr = make(vuRenderer, vuRenderPass, vertexShaderPath, fragmentShaderPath);
  throw_if_unexpected(newVuShaderOrErr);
  *this = std::move(newVuShaderOrErr.value());

  for (auto& setting : currentlyAvailableMatSettings) {

    VuGraphicsPipeline pipline {vuRenderer->vuDevice->device, vuRenderer->globalPipelineLayout,
                                vertexShaderModule,           fragmentShaderModule,
                                vuRenderPass->renderPass,     vuRenderPass->colorBlendAttachmentStates};
    compiledPipelines.emplace(setting, std::move(pipline));
  }
}

Vu::VuGraphicsPipeline&
Vu::VuShader::requestPipeline(MaterialSettings materialSettings) {
  bool contains = compiledPipelines.contains(materialSettings);
  if (!contains) {
    VuGraphicsPipeline pipeline {vuRenderer->vuDevice->device, vuRenderer->globalPipelineLayout,
                                 vertexShaderModule,           fragmentShaderModule,
                                 vuRenderPass->renderPass,     vuRenderPass->colorBlendAttachmentStates};

    compiledPipelines.emplace(materialSettings, std::move(pipeline));
  }
  return compiledPipelines[materialSettings];
}

vk::raii::ShaderModule
Vu::VuShader::createShaderModule(const VuDevice& vuDevice, const void* code, size_t size) {
  vk::ShaderModuleCreateInfo createInfo {};
  createInfo.codeSize = size;
  createInfo.pCode    = static_cast<const u32*>(code);
  createInfo.pNext    = nullptr;

  auto shaderModuleOrErr = vuDevice.device.createShaderModule(createInfo);
  throw_if_unexpected(shaderModuleOrErr);

  return std::move(shaderModuleOrErr.value());
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

  std::string cmd = std::format("{0} {1} -target spirv -fvk-use-scalar-layout  -o {2}", compilerPath.string(),
                                shaderPath.string(), spirvFilePath.string());

  int compileResult = system(cmd.c_str());
  assert(compileResult == 0);
  return spirvFilePath;
}
