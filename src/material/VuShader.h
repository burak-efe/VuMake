#pragma once
#include <vector>
#include "Common.h"
#include "VuMaterial.h"
#include "VuUtils.h"
#include "VuConfig.h"
#include "slang.h"
#include "slang-com-ptr.h"

namespace Vu {


    struct VuGraphicsShaderCreateInfo {
        std::filesystem::path vertexShaderPath;
        std::filesystem::path fragmentShaderPath;
        VkRenderPass renderPass;
    };

    struct VuShader {

        inline static slang::IGlobalSession* globalSession;

        VuGraphicsShaderCreateInfo lastCreateInfo;

        VkShaderModule vertexShaderModule;
        VkShaderModule fragmentShaderModule;
        VkRenderPass renderPass;
        std::vector<VuMaterial> materials;

        time_t lastModifiedTime = 0;

        static void initCompilerSystem() {
            slang::createGlobalSession(&globalSession);

        }

        static void uninitCompilerSystem() {
            slang::shutdown();
        }

        static time_t getlastModifiedTime(const std::filesystem::path& path) {
            const auto fileTime = std::filesystem::last_write_time(path);
            const auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(fileTime);
            const auto time = std::chrono::system_clock::to_time_t(systemTime);
            return time;
        }


        void initAsGraphicsShader(const VuGraphicsShaderCreateInfo& createInfo) {
            ZoneScoped;
            this->lastCreateInfo = createInfo;
            this->renderPass = createInfo.renderPass;

            const std::vector<char> vertShaderCode = Vu::readFile(createInfo.vertexShaderPath);
            const std::vector<char> fragShaderCode = Vu::readFile(createInfo.fragmentShaderPath);

            lastModifiedTime = std::max(getlastModifiedTime(createInfo.vertexShaderPath),
                                        getlastModifiedTime(createInfo.fragmentShaderPath));


            auto vertOutPath = createInfo.vertexShaderPath;
            vertOutPath.replace_extension("spv");
            auto fragOutPath = createInfo.fragmentShaderPath;
            fragOutPath.replace_extension("spv");

            //
            // //slang session
            // const char* searchPaths[] = {"assets/shaders/"};
            // slang::TargetDesc targetDesc = {};
            // targetDesc.format = SLANG_SPIRV;
            // targetDesc.profile = globalSession->findProfile("spirv_1_5");
            // targetDesc.flags = 0;
            //
            //
            // slang::SessionDesc sessionDesc = {};
            // sessionDesc.targets = &targetDesc;
            // sessionDesc.targetCount = 1;
            // sessionDesc.searchPaths = searchPaths;
            // sessionDesc.searchPathCount = 1;
            //
            //
            // std::vector<slang::CompilerOptionEntry> options;
            // options.push_back(
            //     {
            //         slang::CompilerOptionName::EmitSpirvDirectly,
            //         {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
            //     });
            // sessionDesc.compilerOptionEntries = options.data();
            // sessionDesc.compilerOptionEntryCount = options.size();
            //
            // slang::ISession* session;
            // globalSession->createSession(sessionDesc, &session);
            //
            //
            // //slang module
            // slang::IBlob* db{};
            // slang::IModule* module = session->loadModule("hello-world.slang", &db);
            // if (db != nullptr) {
            //     fprintf(stderr, "%s\n", (const char *) db->getBufferPointer());
            //
            // }
            //
            //
            // int32 entrypointCount = module->getDefinedEntryPointCount();
            // slang::IEntryPoint* entryPoint{};
            // module->findEntryPointByName("computeMain", &entryPoint);
            //
            // std::vector<slang::IComponentType *> componentTypes{};
            // componentTypes.push_back(module);
            // componentTypes.push_back(entryPoint);
            //
            //
            // slang::IComponentType* composedProgram;
            // //
            // {
            //     slang::IBlob* diagnosticsBlob;
            //     SlangResult result = session->createCompositeComponentType(componentTypes.data(),
            //                                                                componentTypes.size(),
            //                                                                &composedProgram,
            //                                                                &diagnosticsBlob);
            // }
            //
            //
            // slang::IBlob* spirvCode;
            // //
            // {
            //     slang::IBlob* diagnosticsBlob;
            //     SlangResult result = composedProgram->getEntryPointCode(
            //         0,
            //         0,
            //         &spirvCode,
            //         &diagnosticsBlob);
            // }
            //

            std::string vertCmd = std::format("{0} {1} -target spirv -fvk-use-scalar-layout  -o {2}",
                                              config::SHADER_COMPILER_PATH,
                                              createInfo.vertexShaderPath.generic_string(),
                                              vertOutPath.generic_string());
            auto fragCmd = std::format("{0} {1} -target spirv -fvk-use-scalar-layout -o {2}",
                                       config::SHADER_COMPILER_PATH,
                                       createInfo.fragmentShaderPath.generic_string(),
                                       fragOutPath.generic_string());

            uint32 vRes = system(vertCmd.c_str());
            uint32 fRes = system(fragCmd.c_str());
            assert(vRes == 0);
            assert(fRes == 0);


            const auto vertSpv = Vu::readFile(vertOutPath);
            const auto fragSpv = Vu::readFile(fragOutPath);


            vertexShaderModule = createShaderModule(vertSpv);
            fragmentShaderModule = createShaderModule(fragSpv);
        }

        void uninit() {
            vkDestroyShaderModule(ctx::vuDevice->device, vertexShaderModule, nullptr);
            vkDestroyShaderModule(ctx::vuDevice->device, fragmentShaderModule, nullptr);

            for (auto& material: materials) {
                material.uninit();
            }
        }

        void tryRecompile() {

            auto maxTime = std::max(getlastModifiedTime(lastCreateInfo.vertexShaderPath),
                                    getlastModifiedTime(lastCreateInfo.fragmentShaderPath));
            if (maxTime <= lastModifiedTime) {
                return;
            }

            vkDeviceWaitIdle(ctx::vuDevice->device);
            vkDestroyShaderModule(ctx::vuDevice->device, vertexShaderModule, nullptr);
            vkDestroyShaderModule(ctx::vuDevice->device, fragmentShaderModule, nullptr);

            initAsGraphicsShader(lastCreateInfo);

            for (auto& material: materials) {
                material.recompile({vertexShaderModule, fragmentShaderModule, renderPass});
            }
        }

        //returns material Index
        uint32 createMaterial() {
            VuMaterial material;
            material.init({vertexShaderModule, fragmentShaderModule, renderPass});
            materials.push_back(material);
            return materials.capacity() - 1;
        }


        static VkShaderModule createShaderModule(const std::vector<char>& code) {

            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const uint32 *>(code.data());
            createInfo.pNext = nullptr;

            VkShaderModule shaderModule;
            VkCheck(vkCreateShaderModule(ctx::vuDevice->device, &createInfo, nullptr, &shaderModule));
            return shaderModule;
        }
    };
}
