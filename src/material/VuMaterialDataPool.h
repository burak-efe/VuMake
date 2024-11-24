#pragma once
#include "Common.h"
#include "buddy_alloc.h"
#include "VuBuffer.h"
#include "VuTypes.h"

namespace Vu {

    struct VuMaterialDataPool {

    private:
        const VkDeviceSize MINIMUM_BLOCK_SIZE = 64;
        const VkDeviceSize BLOCK_COUNT = 1024;
        VuBuffer materialDataBuffer;
        VkDeviceAddress deviceAddress;

    public:
        void* buddy_metadata;
        buddy* buddy;

        void init() {
            ZoneScoped;

            materialDataBuffer.init({
                .lenght = BLOCK_COUNT,
                .strideInBytes = MINIMUM_BLOCK_SIZE,
                .vkUsageFlags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                .vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
                .vmaCreateFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            });

            materialDataBuffer.Map();

            //VkDeviceAddress ptrAdrs = (VkDeviceAddress)materialDataBuffer.mapPtr;
            deviceAddress = materialDataBuffer.getDeviceAddress();


            size_t arena_size = MINIMUM_BLOCK_SIZE * BLOCK_COUNT;
            buddy_metadata = malloc(buddy_sizeof_alignment(arena_size, MINIMUM_BLOCK_SIZE));
            buddy = buddy_init_alignment(
                static_cast<unsigned char *>(buddy_metadata),
                static_cast<unsigned char *>(materialDataBuffer.mapPtr),
                arena_size,
                MINIMUM_BLOCK_SIZE);
        }

        VkDeviceAddress mapAddressToBufferDeviceAddress(PBRMaterialData* ptr) {
            uint32 offset = (uint64)materialDataBuffer.mapPtr -  (uint64)ptr;
            return deviceAddress + offset;
        }

        PBRMaterialData* allocMaterialData() {
            void* ptr = buddy_malloc(buddy, sizeof(PBRMaterialData));
            return static_cast<PBRMaterialData *>(ptr);
        }

        void freeMaterialData(PBRMaterialData* ptr) {
            buddy_free(buddy, reinterpret_cast<void *>(ptr));
        }

        //Returns the offset in bytes
        // uint32 allocMaterialData(VkDeviceSize size) {
        //     void* data0 = buddy_malloc(buddy, size);
        //     VkDeviceSize offset = reinterpret_cast<uint64>(data0) - reinterpret_cast<uint64>(materialDataBuffer.mapPtr);
        //     return static_cast<uint32>(offset);
        // }
        //
        // void freeMaterialData(uint32 offset) {
        //
        //     VkDeviceSize ptr = reinterpret_cast<VkDeviceSize>(materialDataBuffer.mapPtr) + offset;
        //     buddy_free(buddy, reinterpret_cast<void *>(ptr));
        // }

        void dispose() {
            materialDataBuffer.Unmap();
            materialDataBuffer.uninit();
            free(buddy_metadata);
        }
    };
}
