#pragma once
#include "Common.h"
#include "VuGlobalSetManager.h"
#include "VuTexture.h"

namespace Vu {
    struct VuTextureHandle {
        uint32 index;

        void init(VuTextureCreateInfo createInfo) {

            index = VuGlobalSetManager::createTexture(createInfo);
        }

        void uninit() {
            decreaseRefCount();
        }

        void increaseRefCount() {
            VuGlobalSetManager::increaseTextureRefCount(index);
        }

        void decreaseRefCount() {
            VuGlobalSetManager::decreaseTextureRefCount(index);
        }

        VuTexture& getRef() {
            return VuGlobalSetManager::allTextures.at(index);
        }
    };

    // struct vuShaderHandle {
    //     uint32 index;
    //
    //     VuShader& getRef() {
    //
    //     }
    // };


}
