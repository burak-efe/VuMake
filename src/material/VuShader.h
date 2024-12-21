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
        inline static slang::ISession* session;

        VuGraphicsShaderCreateInfo lastCreateInfo;

        VkShaderModule vertexShaderModule;
        VkShaderModule fragmentShaderModule;
        VkRenderPass renderPass;
        std::vector<VuMaterial> materials;

        time_t lastModifiedTime = 0;


        void createFile(const std::filesystem::path& path, std::span<const uint8_t> data) {
            // Open file for writing in binary mode
            std::ofstream file(path, std::ios::binary | std::ios::out);

            // Check if the file was successfully opened
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open file: " + path.string());
            }

            // Write data to the file
            file.write(reinterpret_cast<const char *>(data.data()), data.size());

            // Ensure the data was written successfully
            if (!file) {
                throw std::runtime_error("Failed to write to file: " + path.string());
            }
        }

        static void compileToSpirv(const char* shaderName, const char* entryPoitName, slang::IBlob*& outBlob) {
            //slang module
            slang::IBlob* db{};
            slang::IModule* module = session->loadModule(shaderName, &db);
            if (db != nullptr) {
                fprintf(stderr, "%s\n", (const char *) db->getBufferPointer());
            }

            //int32 entrypointCount = module->getDefinedEntryPointCount();
            slang::IEntryPoint* entryPoint{};
            module->findEntryPointByName(entryPoitName, &entryPoint);

            std::vector<slang::IComponentType *> componentTypes{};
            componentTypes.push_back(module);
            componentTypes.push_back(entryPoint);

            slang::IComponentType* composedProgram;
            //
            {
                slang::IBlob* diagnosticsBlob{};
                SlangResult result = session->createCompositeComponentType(componentTypes.data(),
                                                                           componentTypes.size(),
                                                                           &composedProgram,
                                                                           &diagnosticsBlob);
                if (diagnosticsBlob != nullptr) {
                    fprintf(stderr, "%s\n", (const char *) diagnosticsBlob->getBufferPointer());
                }
                assert(result == SLANG_OK);
            }

            //slang::IBlob* spirvCode;
            //
            {
                slang::IBlob* diagnosticsBlob{};
                SlangResult result = composedProgram->getEntryPointCode(0, 0, &outBlob, &diagnosticsBlob);
                if (diagnosticsBlob != nullptr) {
                    fprintf(stderr, "%s\n", (const char *) diagnosticsBlob->getBufferPointer());
                }
                assert(result == SLANG_OK);
            }
        }


        static void initCompilerSystem() {
            slang::createGlobalSession(&globalSession);
            //slang session
            const char* searchPaths[] = {"assets/shaders/"};
            slang::TargetDesc targetDesc {};
            targetDesc.format = SLANG_SPIRV;
            targetDesc.profile = globalSession->findProfile("spirv_1_5");
            targetDesc.flags = 0;
            //targetDesc.forceGLSLScalarBufferLayout = true;
            std::vector<slang::CompilerOptionEntry> targetOptions{};
            // targetOptions.push_back(
            //     {
            //         slang::CompilerOptionName::EmitSpirvDirectly,
            //         {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
            //     }
            // );
            targetOptions.push_back(
                {
                    slang::CompilerOptionName::GLSLForceScalarLayout,
                    {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
                }
            );
            targetOptions.push_back(
                {
                    slang::CompilerOptionName::MatrixLayoutColumn,
                    {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
                }
            );
            //
            // targetOptions.push_back(
            //     {
            //         slang::CompilerOptionName::MatrixLayoutRow,
            //         {slang::CompilerOptionValueKind::Int, 0, 0, nullptr, nullptr}
            //     }
            // );
            targetDesc.compilerOptionEntries = targetOptions.data();
            targetDesc.compilerOptionEntryCount = targetOptions.size();


            slang::SessionDesc sessionDesc {};
            sessionDesc.targets = &targetDesc;
            sessionDesc.targetCount = 1;
            sessionDesc.searchPaths = searchPaths;
            sessionDesc.searchPathCount = 1;
            sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
            std::vector<slang::CompilerOptionEntry> sessionOptions{};
            sessionOptions.push_back(
                {
                    slang::CompilerOptionName::EmitSpirvDirectly,
                    {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
                }
            );
            // sessionOptions.push_back(
            //     {
            //         slang::CompilerOptionName::GLSLForceScalarLayout,
            //         {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
            //     }
            // );
            //
            sessionOptions.push_back(
                {
                    slang::CompilerOptionName::MatrixLayoutColumn,
                    {slang::CompilerOptionValueKind::Int, 1, 0, nullptr, nullptr}
                }
            );
            // sessionOptions.push_back(
            //     {
            //         slang::CompilerOptionName::MatrixLayoutRow,
            //         {slang::CompilerOptionValueKind::Int, 0, 0, nullptr, nullptr}
            //     }
            // );
            sessionDesc.compilerOptionEntries = sessionOptions.data();
            sessionDesc.compilerOptionEntryCount = sessionOptions.size();

            //slang::ISession* session;
            globalSession->createSession(sessionDesc, &session);
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


            lastModifiedTime = std::max(getlastModifiedTime(createInfo.vertexShaderPath),
                                        getlastModifiedTime(createInfo.fragmentShaderPath));


            const std::vector<char> vertShaderCode = Vu::readFile(createInfo.vertexShaderPath);
            const std::vector<char> fragShaderCode = Vu::readFile(createInfo.fragmentShaderPath);
            auto vertOutPath = createInfo.vertexShaderPath;
            vertOutPath.replace_extension("spv");
            auto fragOutPath = createInfo.fragmentShaderPath;
            fragOutPath.replace_extension("spv");


            //  std::string vertCmd = std::format("{0} {1} -target spirv -fvk-use-scalar-layout  -o {2}",
            //                                    config::SHADER_COMPILER_PATH,
            //                                    createInfo.vertexShaderPath.generic_string(),
            //                                    vertOutPath.generic_string());
            //  auto fragCmd = std::format("{0} {1} -target spirv -fvk-use-scalar-layout -o {2}",
            //                             config::SHADER_COMPILER_PATH,
            //                             createInfo.fragmentShaderPath.generic_string(),
            //                             fragOutPath.generic_string());
            //
            //  uint32 vRes = system(vertCmd.c_str());
            //  uint32 fRes = system(fragCmd.c_str());
            //  assert(vRes == 0);
            //  assert(fRes == 0);
            //
            // const auto vertSpv = Vu::readFile(vertOutPath);
            // const auto fragSpv = Vu::readFile(fragOutPath);
            //
            // vertexShaderModule = createShaderModule(vertSpv.data(), vertSpv.size());
            // fragmentShaderModule = createShaderModule(fragSpv.data(), fragSpv.size());


            slang::IBlob* vertSPV{};
            slang::IBlob* fragSPV{};

            compileToSpirv(createInfo.vertexShaderPath.filename().generic_string().c_str(), "vertexMain", vertSPV);
            compileToSpirv(createInfo.fragmentShaderPath.filename().generic_string().c_str(), "fragmentMain", fragSPV);

            createFile("assets/shaders/vertexFromApi.spv", {(uint8_t *) vertSPV->getBufferPointer(), vertSPV->getBufferSize()});
            createFile("assets/shaders/fragmentFromApi.spv", {(uint8_t *) fragSPV->getBufferPointer(), fragSPV->getBufferSize()});

            vertexShaderModule = createShaderModule(vertSPV->getBufferPointer(), vertSPV->getBufferSize());
            fragmentShaderModule = createShaderModule(fragSPV->getBufferPointer(), fragSPV->getBufferSize());
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


        static VkShaderModule createShaderModule(const void* code, size_t size) {

            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = size;
            createInfo.pCode = reinterpret_cast<const uint32 *>(code);
            createInfo.pNext = nullptr;

            VkShaderModule shaderModule;
            VkCheck(vkCreateShaderModule(ctx::vuDevice->device, &createInfo, nullptr, &shaderModule));
            return shaderModule;
        }
    };
}
