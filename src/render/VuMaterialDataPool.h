#pragma once
#include "Common.h"
#include "buddy_alloc.h"
#include "VuBuffer.h"

namespace Vu {

    struct VuMaterialDataPool {

    private:
        const VkDeviceSize MINIMUM_BLOCK_SIZE = 64;
        const VkDeviceSize BLOCK_COUNT = 1024;
        VuBuffer materialDataBuffer;

    public:
        void* buddy_metadata;
        buddy* buddy;

        void Init() {

            materialDataBuffer.Alloc({
                .lenght = BLOCK_COUNT,
                .strideInBytes = MINIMUM_BLOCK_SIZE,
                .vkUsageFlags = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                .vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
                .vmaCreateFlags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
            });

            materialDataBuffer.Map();


            size_t arena_size = MINIMUM_BLOCK_SIZE * BLOCK_COUNT;
            buddy_metadata = malloc(buddy_sizeof_alignment(arena_size, MINIMUM_BLOCK_SIZE));
            //buddy_arena = malloc(arena_size);
            buddy = buddy_init_alignment(
                static_cast<unsigned char *>(buddy_metadata),
                static_cast<unsigned char *>(materialDataBuffer.mapPtr),
                arena_size,
                MINIMUM_BLOCK_SIZE);

            // /* Allocate using the buddy allocator */
            // void* data0 = buddy_malloc(buddy, 32);
            // void* data1 = buddy_malloc(buddy, 32);
            //
            // std::cout << (uint64) data1 - (uint64) data0 << std::endl;
            //
            // /* Free using the buddy allocator */
            // buddy_free(buddy, data0);
            // buddy_free(buddy, data1);

            //free(buddy_metadata);
            //free(buddy_arena);
        }

        //Returns the offset in bytes
        uint32 AllocMaterialData(VkDeviceSize size) {
            void* data0 = buddy_malloc(buddy, size);
            VkDeviceSize offset = reinterpret_cast<uint64>(data0) - reinterpret_cast<uint64>(materialDataBuffer.mapPtr);
            return static_cast<uint32>(offset);
        }

        void FreeMaterialData(uint32 offset) {

            VkDeviceSize ptr = reinterpret_cast<VkDeviceSize>(materialDataBuffer.mapPtr) + offset;
            buddy_free(buddy, reinterpret_cast<void *>(ptr));
        }

        void Dispose() {
            materialDataBuffer.Unmap();
            materialDataBuffer.Dispose();
            free(buddy_metadata);
        }
    };
}
