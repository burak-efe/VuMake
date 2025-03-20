#include "VuShader.h"

#include "VuDevice.h"
#include "11_Config/VuCtx.h"
#include "../10_Core/VuIO.h"

Vu::path Vu::VuShader::compileToSpirv(const path& shaderCodePath)
{
    path spirvFilePath = shaderCodePath;
    spirvFilePath.replace_extension(".spv");

    std::string vertCmd = std::format("{0} {1} -target spirv -fvk-use-scalar-layout  -o {2}",
                                      config::SHADER_COMPILER_PATH,
                                      shaderCodePath.generic_string(),
                                      spirvFilePath.generic_string());

    uint32 compileResult = system(vertCmd.c_str());
    assert(compileResult == 0);
    return spirvFilePath;
}

void Vu::VuShader::init(const VuGraphicsShaderCreateInfo& createInfo)
{
    ZoneScoped;
    this->lastCreateInfo = createInfo;


    lastModifiedTime = std::max(getlastModifiedTime(createInfo.vertexShaderPath),
                                getlastModifiedTime(createInfo.fragmentShaderPath));

    auto vertOutPath = compileToSpirv(createInfo.vertexShaderPath);
    auto fragOutPath = compileToSpirv(createInfo.fragmentShaderPath);

    const auto vertSpv = Vu::readFile(vertOutPath);
    const auto fragSpv = Vu::readFile(fragOutPath);

    vertexShaderModule   = createShaderModule(vertSpv.data(), vertSpv.size());
    fragmentShaderModule = createShaderModule(fragSpv.data(), fragSpv.size());
}

void Vu::VuShader::uninit()
{
    vkDestroyShaderModule(ctx::vuDevice->device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(ctx::vuDevice->device, fragmentShaderModule, nullptr);

    for (auto& material : materials)
    {
        material.uninit();
    }
}

void Vu::VuShader::tryRecompile()
{
    auto maxTime = std::max(getlastModifiedTime(lastCreateInfo.vertexShaderPath),
                            getlastModifiedTime(lastCreateInfo.fragmentShaderPath));
    if (maxTime <= lastModifiedTime)
    {
        return;
    }

    vkDeviceWaitIdle(ctx::vuDevice->device);
    vkDestroyShaderModule(ctx::vuDevice->device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(ctx::vuDevice->device, fragmentShaderModule, nullptr);

    init(lastCreateInfo);

    for (auto& material : materials)
    {
        material.recompile({vertexShaderModule, fragmentShaderModule, lastCreateInfo.renderPass});
    }
}

Vu::uint32 Vu::VuShader::createMaterial(VuMaterialDataPool* materialDataPool)
{
    VuMaterial material;
    material.init({vertexShaderModule, fragmentShaderModule, lastCreateInfo.renderPass, materialDataPool});
    materials.push_back(material);
    return materials.capacity() - 1;
}

VkShaderModule Vu::VuShader::createShaderModule(const void* code, size_t size)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode    = reinterpret_cast<const uint32*>(code);
    createInfo.pNext    = nullptr;

    VkShaderModule shaderModule;
    VkCheck(vkCreateShaderModule(ctx::vuDevice->device, &createInfo, nullptr, &shaderModule));
    return shaderModule;
}
