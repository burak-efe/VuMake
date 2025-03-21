#include "VuShader.h"

#include <iostream>

#include "10_Core/VuIO.h"
#include "11_Config/VuCtx.h"

#include "VuDevice.h"

Vu::Path Vu::VuShader::compileToSpirv(const Path& shaderCodePath)
{
    Path shaderPath = shaderCodePath;
    shaderPath.make_preferred();

    Path spirvFilePath = shaderCodePath;
    spirvFilePath.make_preferred();
    spirvFilePath.replace_extension(".spv");


    Path compilerPath = config::SHADER_COMPILER_PATH;
    compilerPath.make_preferred();


    std::string cmd = std::format("{0} {1} -target spirv -fvk-use-scalar-layout  -o {2}",
                                  compilerPath.string(),
                                  shaderPath.string(),
                                  spirvFilePath.string());


    int compileResult = system(cmd.c_str());
    assert(compileResult == 0);
    return spirvFilePath;
}

void Vu::VuShader::init(VuDevice* vuDevice, Path vertexShaderPath, Path fragmentShaderPath, VkRenderPass renderPass)
{
    ZoneScoped;
    this->vuDevice           = vuDevice;
    this->vertexShaderPath   = vertexShaderPath;
    this->fragmentShaderPath = fragmentShaderPath;
    this->renderPass         = renderPass;


    lastModifiedTime = std::max(getlastModifiedTime(vertexShaderPath),
                                getlastModifiedTime(fragmentShaderPath));

    auto vertOutPath = compileToSpirv(vertexShaderPath);
    auto fragOutPath = compileToSpirv(fragmentShaderPath);

    const auto vertSpv = Vu::readFile(vertOutPath);
    const auto fragSpv = Vu::readFile(fragOutPath);

    fragmentShaderModule = createShaderModule(vuDevice, fragSpv.data(), fragSpv.size());
    vertexShaderModule   = createShaderModule(vuDevice, vertSpv.data(), vertSpv.size());
}

void Vu::VuShader::uninit()
{
    vkDestroyShaderModule(vuDevice->device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(vuDevice->device, fragmentShaderModule, nullptr);

    for (auto& material : materials)
    {
        material.uninit();
    }
}

void Vu::VuShader::tryRecompile()
{
    auto maxTime = std::max(getlastModifiedTime(vertexShaderPath),
                            getlastModifiedTime(fragmentShaderPath));
    if (maxTime <= lastModifiedTime)
    {
        return;
    }

    vkDeviceWaitIdle(vuDevice->device);
    vkDestroyShaderModule(vuDevice->device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(vuDevice->device, fragmentShaderModule, nullptr);

    init(
         vuDevice,
         vertexShaderPath,
         fragmentShaderPath,
         renderPass
        );

    for (auto& material : materials)
    {
        //TODO
        //material.recompile({vertexShaderModule, fragmentShaderModule, lastCreateInfo.renderPass});
    }
}

Vu::uint32 Vu::VuShader::createMaterial()
{
    VuMaterial material;
    material.init(vuDevice, vertexShaderModule, fragmentShaderModule, renderPass);
    materials.push_back(material);
    return materials.capacity() - 1;
}

VkShaderModule Vu::VuShader::createShaderModule(VuDevice* vuDevice, const void* code, size_t size)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode    = static_cast<const uint32*>(code);
    createInfo.pNext    = nullptr;

    VkShaderModule shaderModule;
    VkCheck(vkCreateShaderModule(vuDevice->device, &createInfo, nullptr, &shaderModule));
    return shaderModule;
}
