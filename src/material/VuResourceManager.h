#pragma once
#include <stdexcept>

#include "VuBuffer.h"
#include "VuConfig.h"
#include "VuSampler.h"
#include "VuShader.h"
#include "VuTexture.h"

namespace Vu {
    struct VuSlotAllocator {
    private:
        std::vector<VkBool32> occupiedSlots;

    public:
        void init(uint32 count) {
            occupiedSlots.resize(count);
        }

        uint32 reserve() {
            for (uint32 i = 0; i < occupiedSlots.size(); i++) {
                if (occupiedSlots[i] == VK_FALSE) {
                    occupiedSlots[i] = VK_TRUE;
                    return i;
                }
            }
            throw std::runtime_error("Out of global resource!");
        }

        void free(uint32 index) {
            occupiedSlots[index] = VK_FALSE;
        }
    };

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct VuResourceManager {
        static void init(uint32 storageCount, uint32 imageCount, uint32 samplerCount, uint32 shaderCount, uint32 materialCount) {
            ZoneScoped;

            allMaterials.resize(shaderCount);
            materialRefCounts.resize(shaderCount);
            materialSlotAllocator.init(shaderCount);


            allShaders.resize(shaderCount);
            shaderRefCounts.resize(shaderCount);
            shaderSlotAllocator.init(shaderCount);

            allTextures.resize(imageCount);
            textureRefCounts.resize(imageCount);
            textureSlotAllocator.init(imageCount);

            allSamplers.resize(samplerCount);
            samplerRefCounts.resize(samplerCount);
            samplerSlotAllocator.init(samplerCount);

            allBuffers.resize(storageCount);
            bufferRefCounts.resize(storageCount);
            bufferSlotAllocator.init(storageCount);

            bufferOfBufferPointers.init({storageCount, sizeof(uint64)});
        }

        static void uninit() {
            bufferOfBufferPointers.uninit();
        }
        //////////////////////////////////////////////Material///////////////////////////////////////////////////////////
        inline static std::vector<VuMaterial> allMaterials;
        inline static std::vector<uint32> materialRefCounts;
        inline static VuSlotAllocator materialSlotAllocator;

        static uint32 createMaterial(VuMaterialCreateInfo createInfo) {
            ZoneScoped;
            uint32 index = materialSlotAllocator.reserve();
            allMaterials[index].init(createInfo);
            materialRefCounts[index] = 1;
            return index;
        }

        static void increaseMaterialRefCount(uint32 materialIndex) {
            materialRefCounts[materialIndex]++;
        }

        static void decreaseMaterialRefCount(uint32 materialIndex) {
            materialRefCounts[materialIndex]--;
            if (materialRefCounts[materialIndex] <= 0) {
                allMaterials[materialIndex].uninit();
                materialSlotAllocator.free(materialIndex);
            }
        }
        //////////////////////////////////////////////Shader///////////////////////////////////////////////////////////
        inline static std::vector<VuShader> allShaders;
        inline static std::vector<uint32> shaderRefCounts;
        inline static VuSlotAllocator shaderSlotAllocator;

        static uint32 createShader(VuShaderCreateInfo createInfo) {
            ZoneScoped;
            uint32 index = shaderSlotAllocator.reserve();
            allShaders[index].init(createInfo);
            shaderRefCounts[index] = 1;
            return index;
        }

        static void increaseShaderRefCount(uint32 shaderIndex) {
            shaderRefCounts[shaderIndex]++;
        }

        static void decreaseShaderRefCount(uint32 shaderIndex) {

            shaderRefCounts[shaderIndex]--;

            if (shaderRefCounts[shaderIndex] <= 0) {
                allShaders[shaderIndex].uninit();
                shaderSlotAllocator.free(shaderIndex);
            }
        }

        //////////////////////////////////////////////Texture///////////////////////////////////////////////////////////
        inline static std::vector<VuTexture> allTextures;
        inline static std::vector<uint32> textureRefCounts;
        inline static VuSlotAllocator textureSlotAllocator;

        static uint32 createTexture(VuTextureCreateInfo createInfo) {
            ZoneScoped;
            uint32 index = textureSlotAllocator.reserve();
            allTextures[index].init(createInfo);
            writeTexture(index, allTextures[index]);
            textureRefCounts[index] = 1;
            return index;
        }

        static void increaseTextureRefCount(uint32 textureIndex) {
            textureRefCounts[textureIndex]++;
        }

        static void decreaseTextureRefCount(uint32 textureIndex) {

            textureRefCounts[textureIndex]--;

            if (textureRefCounts[textureIndex] <= 0) {
                allTextures[textureIndex].uninit();
                textureSlotAllocator.free(textureIndex);
            }
        }
        ////////////////////////////////////////////////////Sampler/////////////////////////////////////////////////////
        inline static std::vector<VuSampler> allSamplers;
        inline static std::vector<uint32> samplerRefCounts;
        inline static VuSlotAllocator samplerSlotAllocator;

        static uint32 createSampler(VuSamplerCreateInfo createInfo) {
            ZoneScoped;
            uint32 index = samplerSlotAllocator.reserve();
            allSamplers[index].init(createInfo);
            writeSampler(index, allSamplers[index]);
            samplerRefCounts[index] = 1;
            return index;
        }

        static void increaseSamplerRefCount(uint32 samplerIndex) {
            samplerRefCounts[samplerIndex]++;
        }

        static void decreaseSamplerRefCount(uint32 samplerIndex) {
            samplerRefCounts[samplerIndex]--;
            if (samplerRefCounts[samplerIndex] <= 0) {
                allSamplers[samplerIndex].uninit();
                samplerSlotAllocator.free(samplerIndex);
            }
        }
        ////////////////////////////////////////////////Buffer//////////////////////////////////////////////////////////
        inline static std::vector<VuBuffer> allBuffers;
        inline static std::vector<uint32> bufferRefCounts;
        inline static VuSlotAllocator bufferSlotAllocator;
        inline static VuBuffer bufferOfBufferPointers;

        static uint32 createBuffer(VuBufferCreateInfo createInfo) {
            ZoneScoped;
            uint32 index = bufferSlotAllocator.reserve();
            allBuffers[index].init(createInfo);
            writeBuffer(index, allBuffers[index]);
            bufferRefCounts[index] = 1;
            return index;
        }

        static void increaseBufferRefCount(uint32 bufferIndex) {
            bufferRefCounts[bufferIndex]++;
        }

        static void decreaseBufferRefCount(uint32 bufferIndex) {
            bufferRefCounts[bufferIndex]--;
            if (bufferRefCounts[bufferIndex] <= 0) {
                allBuffers[bufferIndex].uninit();
                bufferSlotAllocator.free(bufferIndex);
            }
        }

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    private:
        static void writeBuffer(uint32 writeIndex, const VuBuffer& buffer) {
            auto address = buffer.getDeviceAddress();
            bufferOfBufferPointers.setDataWithOffset(&address, writeIndex * sizeof(VkDeviceAddress),
                sizeof(VkDeviceAddress));
        }

        static void writeTexture(uint32 writeIndex, const VuTexture& texture) {

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

        static void writeSampler(uint32 writeIndex, const VuSampler& sampler) {

            VkDescriptorImageInfo imageInfo{
                .sampler = sampler.vkSampler,
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

        static uint32 registerTexture(const VuTexture& texture) {
            uint32 index = textureSlotAllocator.reserve();
            writeTexture(index, texture);
            return index;
        }

        static void unregisterTexture(uint32 index) {
            textureSlotAllocator.free(index);
        }

    };


}
