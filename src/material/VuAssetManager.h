#pragma once
#include <unordered_map>

#include "Common.h"
#include "VuResourceManager.h"
#include "VuTexture.h"

namespace Vu {
    struct VuAssetManager {
    public:

        std::set<std::string> loadedTextures;

        //Get Asset, Load if not loaded, return if not exist.
        VuHandle<VuTexture> LoadTextureAsset(const std::string& filePath) {
            if (loadedTextures.contains(filePath)) {

            }

        }
        //resorce states: Loaded, Unloaded, Source: Disk, Web, Procedural
    };
}
