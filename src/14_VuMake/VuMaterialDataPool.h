#pragma once

#include "10_Core/VuCommon.h"
#include "12_VuMakeCore/VuBuffer.h"
#include "12_VuMakeCore/VuTypes.h"
#include "12_VuMakeCore/VuPools.h"

namespace Vu
{
    // struct VuDevice;
    //
    // struct VuMaterialDataPool
    // {
    // private:
    //     VuDevice* vuDevice;
    //     static constexpr VkDeviceSize       MATERIAL_DATA_SIZE = 64;
    //     static constexpr uint32             MAX_MATERIAL_DATA  = 1024;
    //     VuHandle2<VuBuffer>                 materialDataBufferHandle{};
    //     VkDeviceAddress                     deviceAddress{};
    //     VuPool2<uint32, MAX_MATERIAL_DATA>* matPool = nullptr;
    //
    //
    // public:
    //     void init(VuDevice* vuDevice);
    //
    //     void uninit();
    //
    //     VkDeviceAddress mappedPtr_To_BDA(GPU_PBR_MaterialData* ptr);
    //
    //     VuHandle2<uint32> allocMaterialData();
    //
    //     void destroyHandle(VuHandle2<uint32> handle);
    //
    //     GPU_PBR_MaterialData* getMaterialData(VuHandle2<uint32> handle);
    // };
}
