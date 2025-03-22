#pragma once

#include "10_Core/VuCommon.h"
#include "12_VuMakeCore/VuPools.h"




namespace Vu
{
    struct GPU_PBR_MaterialData;
    struct VuShader;
    struct VuDevice;

    struct MaterialSettings
    {

        bool            isTransparent = false;
        VkCullModeFlags cullMode      = VK_CULL_MODE_BACK_BIT;

        friend bool operator==(const MaterialSettings& lhs, const MaterialSettings& rhs)
        {
            return lhs.isTransparent == rhs.isTransparent
                   && lhs.cullMode == rhs.cullMode;
        }

        friend bool operator!=(const MaterialSettings& lhs, const MaterialSettings& rhs)
        {
            return !(lhs == rhs);
        }

        friend std::size_t hash_value(const MaterialSettings& obj)
        {
            std::size_t seed = 0x305407C8;
            seed ^= (seed << 6) + (seed >> 2) + 0x42B03DC4 + static_cast<std::size_t>(obj.isTransparent);
            seed ^= (seed << 6) + (seed >> 2) + 0x29CD679B + static_cast<std::size_t>(obj.cullMode);
            return seed;
        }
    };


    //Material owns the pipeline, uses shared material data
    //when parent shader recompiled, it should be recompiled too
    struct VuMaterial
    {
        VuDevice*           vuDevice;
        MaterialSettings    materialSettings;
        VuHnd<VuShader> shaderHnd;
        VuHnd<uint32>   materialDataHnd;

        // VkShaderModule vertexShaderModule;
        // VkShaderModule fragmentShaderModule;
        // VkRenderPass   renderPass;
        // VuGraphicsPipeline vuPipeline;

        void init(VuDevice* vuDevice, MaterialSettings matSettings, VuHnd<VuShader> shaderHnd, VuHnd<uint32> materialDataHnd);

        //void recompile();

        void uninit();

        //void bindMaterial(const VkCommandBuffer& commandBuffer) const;

        //GPU_PBR_MaterialData* getMaterialData();
    };
}

namespace std {
    template <>
    struct hash<Vu::MaterialSettings> {
        std::size_t operator()(const Vu::MaterialSettings& settings) const {
            std::size_t h1 = std::hash<bool>()(settings.isTransparent);
            std::size_t h2 = std::hash<VkFlags>()(settings.cullMode);

            return h1 ^ (h2 << 1);
        }
    };
}
