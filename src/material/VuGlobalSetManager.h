#pragma once
#include <stdexcept>

#include "VuBuffer.h"
#include "VuConfig.h"
#include "VuSampler.h"
#include "VuTexture.h"

namespace Vu {

    struct VuResourceSlotAllocator {

        void init(uint32 count) {
            occupyList.resize(count);
        }

        uint32 reserve() {
            for (uint32 i = 0; i < occupyList.size(); i++) {
                if (occupyList[i] == VK_FALSE) {
                    occupyList[i] = VK_TRUE;
                    return i;
                }
            }
            throw std::runtime_error("Out of global resource!");
        }

        void free(uint32 index) {
            occupyList[index] = VK_FALSE;
        }

    private:
        std::vector<VkBool32> occupyList;
    };


    struct VuGlobalSetManager {

        void init(uint32 storageCount , uint32 imageCount , uint32 samplerCount) {
            bufferOfBufferPointers.Alloc({storageCount,sizeof(uint64)});
            bufferSlotAllocator.init(storageCount);
            textureSlotAllocator.init(imageCount);
            samplerSlotAllocator.init(samplerCount);
        }

        void dispose() {
            bufferOfBufferPointers.Dispose();
        }



        //register to global buffer and get index
        uint32 registerBuffer(const VuBuffer& buffer) {
            auto index = bufferSlotAllocator.reserve();
            writeBuffer(index, buffer);
            return index;
        }

        void unregisterBuffer(uint32 index) {
            bufferSlotAllocator.free(index);
        }

        //Texture
        uint32 registerTexture(const VuTexture& texture) {
            auto index = textureSlotAllocator.reserve();
            writeTexture(index, texture);
            return index;
        }

        void unregisterTexture(uint32 index) {
            textureSlotAllocator.free(index);
        }

        //Sampler
        uint32 registerSampler(const VuSampler& sampler) {
            auto index = samplerSlotAllocator.reserve();
            writeSampler(index, sampler.vkSampler);
            return index;
        }

        void unregisterSampler(uint32 index) {
            samplerSlotAllocator.free(index);
        }




    private:
        VuResourceSlotAllocator bufferSlotAllocator;
        VuResourceSlotAllocator samplerSlotAllocator;
        VuResourceSlotAllocator textureSlotAllocator;

        VuBuffer bufferOfBufferPointers;

        void writeBuffer(uint32 writeIndex, const VuBuffer& buffer) {
            auto address = buffer.getDeviceAddress();
            bufferOfBufferPointers.SetDataWithOffset(&address, writeIndex * sizeof(VkDeviceAddress), sizeof(VkDeviceAddress));
        }

        void writeTexture(uint32 writeIndex, const VuTexture& texture) {

            VkDescriptorImageInfo imageInfo{
                .sampler = VK_NULL_HANDLE,
                .imageView = texture.textureImageView,
                .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            };

            for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
                VkWriteDescriptorSet descriptorWrite{};
                descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrite.dstSet = ctx::globalDescriptorSets[i];
                descriptorWrite.dstBinding = config::IMAGE_BINDING;
                descriptorWrite.dstArrayElement = writeIndex;
                descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
                descriptorWrite.descriptorCount = 1;
                descriptorWrite.pImageInfo = &imageInfo;
                vkUpdateDescriptorSets(ctx::device, 1, &descriptorWrite, 0, nullptr);
            }
        }

        void writeSampler(uint32 writeIndex, const VkSampler& sampler) {

            VkDescriptorImageInfo imageInfo{
                .sampler = sampler,
            };

            for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
                VkWriteDescriptorSet samplerWrite{};
                samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                samplerWrite.dstSet = ctx::globalDescriptorSets[i];
                samplerWrite.dstBinding = config::SAMPLER_BINDING;
                samplerWrite.dstArrayElement = writeIndex;
                samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
                samplerWrite.descriptorCount = 1;
                samplerWrite.pImageInfo = &imageInfo;
                vkUpdateDescriptorSets(ctx::device, 1, &samplerWrite, 0, nullptr);
            }
        }

    };
}
