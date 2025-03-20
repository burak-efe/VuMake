#include "VuMaterial.h"

#include "11_Config/VuCtx.h"
#include "VuDevice.h"
#include "VuMaterialDataPool.h"
#include "VuMesh.h"

void Vu::VuMaterial::init(const VuMaterialCreateInfo& createInfo)
{
    lastCreateInfo = createInfo;
    auto bindings = VuMesh::getBindingDescription();
    auto attribs  = VuMesh::getAttributeDescriptions();
    vuPipeline.initGraphicsPipeline(
                                    ctx::vuDevice->globalPipelineLayout,
                                    createInfo.vertexShaderModule,
                                    createInfo.fragmentShaderModule,
                                    bindings,
                                    attribs,
                                    createInfo.renderPass
                                   );

    materialData = createInfo.materialDataPool->allocMaterialData();
}

void Vu::VuMaterial::recompile(const VuMaterialCreateInfo& createInfo)
{
    vuPipeline.Dispose();

    auto bindings = VuMesh::getBindingDescription();
    auto attribs  = VuMesh::getAttributeDescriptions();
    vuPipeline.initGraphicsPipeline(
                                    ctx::vuDevice->globalPipelineLayout,
                                    createInfo.vertexShaderModule,
                                    createInfo.fragmentShaderModule,
                                    bindings,
                                    attribs,
                                    createInfo.renderPass
                                   );

}

void Vu::VuMaterial::uninit()
{
    vuPipeline.Dispose();
    lastCreateInfo.materialDataPool->destroyHandle(materialData);
}

void Vu::VuMaterial::bindPipeline(const VkCommandBuffer& commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vuPipeline.pipeline);
}

Vu::GPU_PBR_MaterialData* Vu::VuMaterial::getMaterialData()
{
    return lastCreateInfo.materialDataPool->getMaterialData(materialData);
}
