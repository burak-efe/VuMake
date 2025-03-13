#include "VuMaterial.h"

#include "VuCtx.h"
#include "VuDevice.h"
#include "VuMaterialDataPool.h"
#include "VuMesh.h"

void Vu::VuMaterial::init(const VuMaterialCreateInfo& createInfo)
{
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

    pbrMaterialData = VuMaterialDataPool::allocMaterialData();
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
    VuMaterialDataPool::freeMaterialData(pbrMaterialData);
}

void Vu::VuMaterial::bindPipeline(const VkCommandBuffer& commandBuffer) const
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vuPipeline.pipeline);
}
