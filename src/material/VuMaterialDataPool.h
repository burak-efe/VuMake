#pragma once
#include "Common.h"

#include "VuBuffer.h"
#include "VuTypes.h"
#include "VuPools.h"

namespace Vu
{
    struct VuMaterialDataPool
    {
    private:
        static constexpr VkDeviceSize       MATERIAL_DATA_SIZE = 64;
        static constexpr uint32             MAX_MATERIAL_DATA  = 1024;
        VuHandle2<VuBuffer>                 materialDataBufferHandle{};
        VkDeviceAddress                     deviceAddress{};
        VuPool2<uint32, MAX_MATERIAL_DATA>* matPool = nullptr;

        VuPool2<VuBuffer, 32>* bufferPool = nullptr;

    public:
        //init the tracker pool and GPU buffer that material data reside
        void init(VuPool2<VuBuffer, 32>* pool);

        void uninit();

        VkDeviceAddress mappedPtr_To_BDA(GPU_PBR_MaterialData* ptr);

        VuHandle2<uint32> allocMaterialData();

        void destroyHandle(VuHandle2<uint32> handle);

        GPU_PBR_MaterialData* getMaterialData(VuHandle2<uint32> handle);
    };
}
