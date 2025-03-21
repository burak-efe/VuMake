#include "VuMaterialDataPool.h"

#include <iostream>

#include "VuDevice.h"

void Vu::VuMaterialDataPool::init(VuDevice* vuDevice)
{
    this->vuDevice           = vuDevice;
    materialDataBufferHandle = vuDevice->createBuffer({
                                                          .length = MAX_MATERIAL_DATA,
                                                          .strideInBytes = MATERIAL_DATA_SIZE,
                                                          .vkUsageFlags =
                                                          VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                                                          .vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
                                                          .vmaCreateFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                                            VMA_ALLOCATION_CREATE_MAPPED_BIT,
                                                      });
    assert(materialDataBufferHandle.index == 1);
    VuBuffer* matDataBuffer = vuDevice->get(materialDataBufferHandle);
    // materialDataBufferHandle = pool->createHandle();
    // VuBuffer* matDataBuffer = pool->get(materialDataBufferHandle);
    //
    //
    // matDataBuffer->init({
    //                         .length = MAX_MATERIAL_DATA,
    //                         .strideInBytes = MATERIAL_DATA_SIZE,
    //                         .vkUsageFlags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
    //                         .vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
    //                         .vmaCreateFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
    //                                           VMA_ALLOCATION_CREATE_MAPPED_BIT,
    //                     });

    matDataBuffer->map();
    deviceAddress = matDataBuffer->getDeviceAddress();
    //VuResourceManager::registerStorageBuffer(*matDataBuffer, materialDataBufferHandle.index);
    matPool = new VuPool2<uint32, MAX_MATERIAL_DATA>();
}

void Vu::VuMaterialDataPool::uninit()
{
    vuDevice->destroyHandle(materialDataBufferHandle);
}

VkDeviceAddress Vu::VuMaterialDataPool::mappedPtr_To_BDA(GPU_PBR_MaterialData* ptr)
{
    VuBuffer* matDataBuffer = vuDevice->get(materialDataBufferHandle);
    uint32    offset        = reinterpret_cast<uint64>(matDataBuffer->mapPtr) - reinterpret_cast<uint64>(ptr);
    return deviceAddress + offset;
}

Vu::VuHandle2<uint32_t> Vu::VuMaterialDataPool::allocMaterialData()
{
    VuHandle2<uint32_t> handle = matPool->createHandle();
    return handle;
}


void Vu::VuMaterialDataPool::destroyHandle(VuHandle2<uint32> handle)
{
    bool isFreed = matPool->destroyHandle(handle);
    if (isFreed)
    {
        std::cout << "material destroyed" << std::endl;
    }
}

Vu::GPU_PBR_MaterialData* Vu::VuMaterialDataPool::getMaterialData(VuHandle2<uint32> handle)
{
    VuBuffer* matDataBuffer = vuDevice->get(materialDataBufferHandle);
    byte*     dataPtr       = static_cast<byte*>(matDataBuffer->mapPtr) + MATERIAL_DATA_SIZE * handle.index;
    return reinterpret_cast<GPU_PBR_MaterialData*>(dataPtr);
}
