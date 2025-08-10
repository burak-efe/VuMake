#include "VuRenderPass.h"

void
Vu::VuRenderPass::initAsGBufferPass(std::shared_ptr<VuDevice> vuDevice,
                                    const VkFormat            colorFormat,
                                    const VkFormat            normalFormat,
                                    const VkFormat            aoRoughMetalFormat,
                                    const VkFormat            worldPosFormat,
                                    const VkFormat            depthStencilFormat) {
  this->m_vuDevice                          = vuDevice;
  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format                  = colorFormat;
  colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout             = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

  VkAttachmentDescription normalAttachment = colorAttachment;
  normalAttachment.format                  = normalFormat;

  VkAttachmentDescription armAttachment = colorAttachment;
  armAttachment.format                  = aoRoughMetalFormat;

  VkAttachmentDescription worldPosAttachment = colorAttachment;
  worldPosAttachment.format                  = worldPosFormat;

  VkAttachmentDescription depthAttachment = {};
  depthAttachment.format                  = depthStencilFormat;
  depthAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
  depthAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depthAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
  depthAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depthAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_STORE;
  depthAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
  depthAttachment.finalLayout             = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkAttachmentDescription attachments[] = {
      colorAttachment, normalAttachment, armAttachment, worldPosAttachment, depthAttachment};

  VkAttachmentReference colorRefs[] = {{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                       {1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                       {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                                       {3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};

  VkAttachmentReference depthRef = {.attachment = 4, .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};

  VkSubpassDescription subpass    = {};
  subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount    = 4;
  subpass.pColorAttachments       = colorRefs;
  subpass.pDepthStencilAttachment = &depthRef;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass          = 0;
  dependency.srcStageMask        = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
  dependency.dependencyFlags = 0;

  VkRenderPassCreateInfo renderPassInfo = {.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  renderPassInfo.attachmentCount        = 5;
  renderPassInfo.pAttachments           = attachments;
  renderPassInfo.subpassCount           = 1;
  renderPassInfo.pSubpasses             = &subpass;
  renderPassInfo.dependencyCount        = 1;
  renderPassInfo.pDependencies          = &dependency;

  VkResult rpRes = vkCreateRenderPass(this->m_vuDevice->m_device, &renderPassInfo, NO_ALLOC_CALLBACK, &this->m_renderPass);
  THROW_if_fail(rpRes);

  m_colorBlendAttachmentStates.resize(4);
  for (auto& blendAttachment : m_colorBlendAttachmentStates) {
    blendAttachment.blendEnable = VK_FALSE; // No blending in GBuffer
    blendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  }
}

void
Vu::VuRenderPass::initAsLightningPass(std::shared_ptr<VuDevice> vuDevice, VkFormat colorFormat) {
  this->m_vuDevice = vuDevice;

  VkAttachmentDescription colorAttachment = {};
  colorAttachment.format                  = colorFormat;
  colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
  colorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  colorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  colorAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
  colorAttachment.finalLayout             = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

  VkAttachmentReference colorAttachmentRef = {};
  colorAttachmentRef.attachment            = 0;
  colorAttachmentRef.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass    = {};
  subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.colorAttachmentCount    = 1;
  subpass.pColorAttachments       = &colorAttachmentRef;
  subpass.inputAttachmentCount    = 0;
  subpass.pInputAttachments       = NULL;
  subpass.pResolveAttachments     = NULL;
  subpass.pDepthStencilAttachment = NULL;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments    = NULL;

  VkSubpassDependency dependency = {};
  dependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
  dependency.dstSubpass          = 0;
  dependency.srcStageMask        = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  dependency.dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
  dependency.srcAccessMask = 0;
  dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependency.dependencyFlags = 0;

  std::array<VkAttachmentDescription, 1> attachments = {colorAttachment};

  VkRenderPassCreateInfo renderPassInfo {.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
  renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
  renderPassInfo.pAttachments    = attachments.data();
  renderPassInfo.subpassCount    = 1;
  renderPassInfo.pSubpasses      = &subpass;
  renderPassInfo.dependencyCount = 1;
  renderPassInfo.pDependencies   = &dependency;

  VkResult rpRes = vkCreateRenderPass(this->m_vuDevice->m_device, &renderPassInfo, NO_ALLOC_CALLBACK, &this->m_renderPass);
  THROW_if_fail(rpRes);

  m_colorBlendAttachmentStates.resize(1);
  for (auto& blendAttachment : m_colorBlendAttachmentStates) {
    blendAttachment.blendEnable = VK_FALSE; // No blending in GBuffer
    blendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
  }
}