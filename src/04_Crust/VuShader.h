#pragma once
#include "02_OuterCore/Common.h"
#include "03_Mantle/VuGraphicsPipeline.h"
#include "VuMaterial.h"

namespace Vu {
struct VuRenderer;
struct VuRenderPass;
struct VuDevice;

struct VuShader {
  std::shared_ptr<VuRenderer>                              m_vuRenderer {};
  std::shared_ptr<VuRenderPass>                            m_vuRenderPass {};
  path                                                     m_vertexShaderPath {"error"};
  path                                                     m_fragmentShaderPath {"error"};
  VkShaderModule                                           m_vertexShaderModule {nullptr};   // owned
  VkShaderModule                                           m_fragmentShaderModule {nullptr}; // owned
  time_t                                                   m_lastModifiedTime {0};
  std::unordered_map<MaterialSettings, VuGraphicsPipeline> m_compiledPipelines {};

  SETUP_EXPECTED_WRAPPER(VuShader,
                         (std::shared_ptr<VuRenderer>   vuRenderer,
                          std::shared_ptr<VuRenderPass> vuRenderPass,
                          path                          vertexShaderPath,
                          path                          fragmentShaderPath),
                         (vuRenderer, vuRenderPass, vertexShaderPath, fragmentShaderPath))

  void
  tryRecompile();

  // get compiled pipeline handle if present, id not, compile and get
  VuGraphicsPipeline&
  requestPipeline(MaterialSettings materialSettings);

  static path
  compileToSpirv(const path& shaderCodePath);

  static VkShaderModule
  createShaderModule(const VuDevice& vuDevice, const void* code, size_t size);
  //--------------------------------------------------------------------------------------------------------------------
  VuShader();
  VuShader(const VuShader&) = delete;
  VuShader&
  operator=(const VuShader&) = delete;

  VuShader(VuShader&& other) noexcept;

  VuShader&
  operator=(VuShader&& other) noexcept;

  ~VuShader();

private:
  void
  cleanup();
  //--------------------------------------------------------------------------------------------------------------------

  VuShader(std::shared_ptr<VuRenderer>   vuRenderer,
           std::shared_ptr<VuRenderPass> vuRenderPass,
           path                          vertexShaderPath,
           path                          fragmentShaderPath);
};
} // namespace Vu
