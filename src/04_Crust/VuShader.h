#pragma once
#include "02_OuterCore/Common.h"
#include "03_Mantle/VuGraphicsPipeline.h"
#include "VuMaterial.h"

namespace Vu {

struct VuRenderer;
struct VuRenderPass;
struct VuDevice;

struct VuGraphicsShaderCreateInfo {
  path           vertexShaderPath;
  path           fragmentShaderPath;
  vk::RenderPass renderPass;
};

struct VuShader {
  std::shared_ptr<VuRenderer>                              vuRenderer           = {};
  std::shared_ptr<VuRenderPass>                            vuRenderPass         = {};
  path                                                     vertexShaderPath     = {"error"};
  path                                                     fragmentShaderPath   = {"error"};
  vk::raii::ShaderModule                                   vertexShaderModule   = {nullptr};
  vk::raii::ShaderModule                                   fragmentShaderModule = {nullptr};
  time_t                                                   lastModifiedTime     = {0};
  std::unordered_map<MaterialSettings, VuGraphicsPipeline> compiledPipelines    = {};

  static std::expected<VuShader, vk::Result>
  make(const std::shared_ptr<VuRenderer>&   vuRenderer,
       const std::shared_ptr<VuRenderPass>& vuRenderPass,
       const path&                          vertexShaderPath,
       const path&                          fragmentShaderPath);

  void
  tryRecompile();

  // get compiled pipeline handle if present, id not, compile and get
  VuGraphicsPipeline&
  requestPipeline(MaterialSettings materialSettings);

  static path
  compileToSpirv(const path& shaderCodePath);

  static vk::raii::ShaderModule
  createShaderModule(const VuDevice& vuDevice, const void* code, size_t size);

private:
  VuShader(std::shared_ptr<VuRenderer>   vuRenderer,
           std::shared_ptr<VuRenderPass> vuRenderPass,
           path                          vertexShaderPath,
           path                          fragmentShaderPath);
};
} // namespace Vu
