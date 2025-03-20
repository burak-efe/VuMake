#include "VuMaterial.h"

#include "11_Config/VuCtx.h"
#include "VuDevice.h"

void Vu::VuMaterial::init(VuDevice*    vuDevice, VkShaderModule vertexShaderModule, VkShaderModule fragmentShaderModule,
                          VkRenderPass renderPass)
{
    this->vuDevice             = vuDevice;
    this->vertexShaderModule   = vertexShaderModule;
    this->fragmentShaderModule = fragmentShaderModule;
    this->renderPass           = renderPass;

    vuPipeline.initGraphicsPipeline(
                                    vuDevice->device,
                                    vuDevice->globalPipelineLayout,
                                    vertexShaderModule,
                                    fragmentShaderModule,
                                    renderPass
                                   );

    materialData = vuDevice->materialDataPool.allocMaterialData();
}

// void Vu::VuMaterial::recompile(const VuMaterialCreateInfo& createInfo)
// {
//     vuPipeline.uninit();
//     vuPipeline.initGraphicsPipeline(
//                                     ctx::vuDevice->globalPipelineLayout,
//                                     createInfo.vertexShaderModule,
//                                     createInfo.fragmentShaderModule,
//                                     bindings,
//                                     attribs,
//                                     createInfo.renderPass
//                                    );
// }

void Vu::VuMaterial::uninit()
{
    vuPipeline.uninit();
    vuDevice->materialDataPool.destroyHandle(materialData);
}

void Vu::VuMaterial::bindPipeline(const VkCommandBuffer& commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vuPipeline.pipeline);
}

Vu::GPU_PBR_MaterialData* Vu::VuMaterial::getMaterialData()
{
    return vuDevice->materialDataPool.getMaterialData(materialData);
}
