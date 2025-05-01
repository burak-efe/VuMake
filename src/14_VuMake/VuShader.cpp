#include "VuShader.h"

#include <iostream>

#include "10_Core/VuIO.h"
#include "11_Config/VuCtx.h"
#include "11_Config/VuConfig.h"
#include "12_VuMakeCore/VuGraphicsPipeline.h"

#include "VuDevice.h"
#include "../08_LangUtils/VuLogger.h"
#include "12_VuMakeCore/VuRenderPass.h"

void Vu::VuShader::init(VuDevice* vuDevice, path vertexShaderPath, path fragmentShaderPath, VuRenderPass* vuRenderPass)
{
    ZoneScoped;
    this->vuDevice           = vuDevice;
    this->vertexShaderPath   = vertexShaderPath;
    this->fragmentShaderPath = fragmentShaderPath;
    this->vuRenderPass       = vuRenderPass;


    lastModifiedTime = std::max(getlastModifiedTime(vertexShaderPath),
                                getlastModifiedTime(fragmentShaderPath));

    auto vertOutPath = compileToSpirv(vertexShaderPath);
    auto fragOutPath = compileToSpirv(fragmentShaderPath);

    const auto vertSpv = Vu::readFile(vertOutPath);
    const auto fragSpv = Vu::readFile(fragOutPath);

    if (vertSpv.has_value())
    {
        vertexShaderModule = createShaderModule(vuDevice, vertSpv.value().data(), vertSpv.value().size());
    }
    else
    {
        Logger::Error("vertex spv file cannot be read!");
    }

    if (fragSpv.has_value())
    {
        fragmentShaderModule = createShaderModule(vuDevice, fragSpv.value().data(), fragSpv.value().size());
    }
    else
    {
        Logger::Error("frag spv file cannot be read!");
    }
}

void Vu::VuShader::uninit()
{
    vkDestroyShaderModule(vuDevice->device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(vuDevice->device, fragmentShaderModule, nullptr);

    for (auto& pipeline : compiledPipelines)
    {
        pipeline.second.uninit();
    }
}


void Vu::VuShader::tryRecompile()
{
    auto maxTime = std::max(getlastModifiedTime(vertexShaderPath), getlastModifiedTime(fragmentShaderPath));
    if (maxTime <= lastModifiedTime)
    {
        return;
    }

    vkDeviceWaitIdle(vuDevice->device);
    uninit();
    init(vuDevice, vertexShaderPath, fragmentShaderPath, vuRenderPass);

    for (auto& pair : compiledPipelines)
    {
        //pair.second.uninit();
        pair.second.initGraphicsPipeline(
                                         vuDevice->device,
                                         vuDevice->globalPipelineLayout,
                                         vertexShaderModule,
                                         fragmentShaderModule,
                                         vuRenderPass->renderPass,
                                         vuRenderPass->colorBlendAttachmentStates
                                        );
    }
}

Vu::VuGraphicsPipeline& Vu::VuShader::requestPipeline(MaterialSettings materialSettings)
{
    bool contains = compiledPipelines.contains(materialSettings);
    if (!contains)
    {
        VuGraphicsPipeline pipeline;
        pipeline.initGraphicsPipeline(vuDevice->device,
                                      vuDevice->globalPipelineLayout,
                                      vertexShaderModule,
                                      fragmentShaderModule,
                                      vuRenderPass->renderPass,
                                      vuRenderPass->colorBlendAttachmentStates);
        compiledPipelines.emplace(materialSettings, pipeline);
    }
    return compiledPipelines[materialSettings];
}

VkShaderModule Vu::VuShader::createShaderModule(VuDevice* vuDevice, const void* code, size_t size)
{
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = size;
    createInfo.pCode    = static_cast<const u32*>(code);
    createInfo.pNext    = nullptr;

    VkShaderModule shaderModule;
    VkCheck(vkCreateShaderModule(vuDevice->device, &createInfo, nullptr, &shaderModule));
    return shaderModule;
}

path Vu::VuShader::compileToSpirv(const path& shaderCodePath)
{
    path shaderPath = shaderCodePath;
    shaderPath.make_preferred();

    path spirvFilePath = shaderCodePath;
    spirvFilePath.make_preferred();
    spirvFilePath.replace_extension(".spv");


    path compilerPath = config::SHADER_COMPILER_PATH;
    compilerPath.make_preferred();


    std::string cmd = std::format("{0} {1} -target spirv -fvk-use-scalar-layout  -o {2}",
                                  compilerPath.string(),
                                  shaderPath.string(),
                                  spirvFilePath.string());


    int compileResult = system(cmd.c_str());
    assert(compileResult == 0);
    return spirvFilePath;
}
