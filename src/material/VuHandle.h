#pragma once

#include "Common.h"
#include "VuResourceManager.h"
#include "VuTexture.h"

namespace Vu {
    struct VuTextureHandle {
        uint32 index;

        void init(const VuTextureCreateInfo& createInfo) {
            index = VuResourceManager::createTexture(createInfo);
        }

        void uninit() {
            decreaseRefCount();
        }

        void increaseRefCount() {
            VuResourceManager::increaseTextureRefCount(index);
        }

        void decreaseRefCount() {
            VuResourceManager::decreaseTextureRefCount(index);
        }

        VuTexture& getByRef() {
            return VuResourceManager::allTextures.at(index);
        }
    };

    struct VuSamplerHandle {
        uint32 index;

        void init(const VuSamplerCreateInfo& createInfo) {
            index = VuResourceManager::createSampler(createInfo);
        }

        void uninit() {
            decreaseRefCount();
        }

        void increaseRefCount() {
            VuResourceManager::increaseSamplerRefCount(index);
        }

        void decreaseRefCount() {
            VuResourceManager::decreaseSamplerRefCount(index);
        }

        VuSampler& getByRef() {
            return VuResourceManager::allSamplers.at(index);
        }
    };

    struct VuBufferHandle {
        uint32 index;

        void init(const VuBufferCreateInfo& createInfo) {
            index = VuResourceManager::createBuffer(createInfo);
        }

        void uninit() {
            decreaseRefCount();
        }

        void increaseRefCount() {
            VuResourceManager::increaseBufferRefCount(index);
        }

        void decreaseRefCount() {
            VuResourceManager::decreaseBufferRefCount(index);
        }

        VuBuffer& getByRef() {
            return VuResourceManager::allBuffers.at(index);
        }
    };

    struct VuShaderHandle {
        uint32 index;

        void init(const VuShaderCreateInfo& createInfo) {
            index = VuResourceManager::createShader(createInfo);
        }

        void uninit() {
            decreaseRefCount();
        }

        void increaseRefCount() {
            VuResourceManager::increaseShaderRefCount(index);
        }

        void decreaseRefCount() {
            VuResourceManager::decreaseShaderRefCount(index);
        }

        VuShader& getByRef() {
            return VuResourceManager::allShaders.at(index);
        }
    };

    struct VuMaterialHandle {
        uint32 index;

        void init(const VuMaterialCreateInfo& createInfo) {
            index = VuResourceManager::createMaterial(createInfo);
        }

        void uninit() {
            decreaseRefCount();
        }

        void increaseRefCount() {
            VuResourceManager::increaseMaterialRefCount(index);
        }

        void decreaseRefCount() {
            VuResourceManager::decreaseMaterialRefCount(index);
        }

        VuMaterial& getByRef() {
            return VuResourceManager::allMaterials.at(index);
        }
    };

}
