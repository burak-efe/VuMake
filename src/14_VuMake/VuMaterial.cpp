#include "VuMaterial.h"

#include "11_Config/VuCtx.h"
#include "VuDevice.h"

void Vu::VuMaterial::init(VuDevice* vuDevice, MaterialSettings matSettings, VuHnd<VuShader> shaderHnd, VuHnd<uint32> materialDataHnd
                          //VkShaderModule vertexShaderModule,
                          //VkShaderModule fragmentShaderModule,
                          //VkRenderPass   renderPass
)
{
    this->vuDevice         = vuDevice;
    this->materialSettings = matSettings;
    this->shaderHnd        = shaderHnd;
    this->materialDataHnd  = materialDataHnd;

    vuDevice->shaderPool.increaseRefCount(shaderHnd.index);
    vuDevice->materialDataIndexPool.increaseRefCount(materialDataHnd.index);

    auto unused = vuDevice->getShader(shaderHnd)->requestPipeline(materialSettings);


    // this->vuDevice             = vuDevice;
    // this->vertexShaderModule   = vertexShaderModule;
    // this->fragmentShaderModule = fragmentShaderModule;
    // this->renderPass           = renderPass;
    //
    // vuPipeline.initGraphicsPipeline(
    //                                 vuDevice->device,
    //                                 vuDevice->globalPipelineLayout,
    //                                 vertexShaderModule,
    //                                 fragmentShaderModule,
    //                                 renderPass
    //                                );
    //
    // materialData = vuDevice->materialDataPool.allocMaterialData();
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
    vuDevice->destroyHandle(materialDataHnd);
    vuDevice->destroyHandle(shaderHnd);
    //
    //
    // vuDevice->destroyHandle(materialDataHnd);vuPipeline.uninit();
    // vuDevice->destroyHandle(materialDataHnd);vuPipeline.uninit();
    // vuDevice->materialDataIndexPool.destroyHandle(materialData);
}

// void Vu::VuMaterial::bindMaterial(const VkCommandBuffer& commandBuffer) const
// {
//     vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vuPipeline.pipeline);
// }

// Vu::GPU_PBR_MaterialData* Vu::VuMaterial::getMaterialData()
// {
//     VuBuffer* matDataBuffer = vuDevice->get(materialDataBufferHandle);
//     byte*     dataPtr       = static_cast<byte*>(matDataBuffer->mapPtr) + MATERIAL_DATA_SIZE * handle.index;
//     return reinterpret_cast<GPU_PBR_MaterialData*>(dataPtr);
//
//     return vuDevice->materialDataIndexPool.getMaterialData(materialData);
// }
