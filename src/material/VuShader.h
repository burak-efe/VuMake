#pragma once
#include <vector>
#include "Common.h"
#include "VuMaterial.h"
#include "VuUtils.h"
#include "VuConfig.h"
#include "slang.h"
#include "slang-com-ptr.h"

namespace Vu {

    struct VuShaderCreateInfo {
        std::filesystem::path vertexShaderPath;
        std::filesystem::path fragmentShaderPath;
        VkRenderPass renderPass;
    };

    struct VuShader {

        inline static Slang::ComPtr<slang::IGlobalSession> globalSession;

        VuShaderCreateInfo lastCreateInfo;

        VkShaderModule vertexShaderModule;
        VkShaderModule fragmentShaderModule;
        VkRenderPass renderPass;
        std::vector<VuMaterial> materials;

        time_t lastModifiedTime = 0;

        static void initSystem() {
            slang::createGlobalSession(globalSession.writeRef());

        }

        static void uninitSystem() {
            slang::shutdown();
        }

        static time_t getlastModifiedTime(const std::filesystem::path& path) {
            const auto fileTime = std::filesystem::last_write_time(path);
            const auto systemTime = std::chrono::clock_cast<std::chrono::system_clock>(fileTime);
            const auto time = std::chrono::system_clock::to_time_t(systemTime);
            return time;
        }


        void init(const VuShaderCreateInfo& createInfo) {
            ZoneScoped;
            this->lastCreateInfo = createInfo;
            this->renderPass = createInfo.renderPass;

            const auto vertShaderCode = Vu::ReadFile(createInfo.vertexShaderPath);
            const auto fragShaderCode = Vu::ReadFile(createInfo.fragmentShaderPath);

            lastModifiedTime = std::max(getlastModifiedTime(createInfo.vertexShaderPath),
                                        getlastModifiedTime(createInfo.fragmentShaderPath));


            auto vertOutPath = createInfo.vertexShaderPath;
            vertOutPath.replace_extension("spv");
            auto fragOutPath = createInfo.fragmentShaderPath;
            fragOutPath.replace_extension("spv");


            // slang::SessionDesc sessionDesc;
            // Slang::ComPtr<slang::ISession> session;
            // globalSession->createSession(sessionDesc, session.writeRef());
            //
            // slang::IModule* module = session->loadModule("MyShaders");



            std::string vertCmd = std::format("{0} {1} -target spirv  -o {2}",
                                              config::SHADER_COMPILER_PATH,
                                              createInfo.vertexShaderPath.generic_string(),
                                              vertOutPath.generic_string());
            auto fragCmd = std::format("{0} {1} -target spirv  -o {2}",
                                       config::SHADER_COMPILER_PATH,
                                       createInfo.fragmentShaderPath.generic_string(),
                                       fragOutPath.generic_string());

            uint32 vRes = system(vertCmd.c_str());
            uint32 fRes = system(fragCmd.c_str());
            assert(vRes == 0);
            assert(fRes == 0);


            const auto vertSpv = Vu::ReadFile(vertOutPath);
            const auto fragSpv = Vu::ReadFile(fragOutPath);


            vertexShaderModule = createShaderModule(vertSpv);
            fragmentShaderModule = createShaderModule(fragSpv);
        }

        void uninit() {
            vkDestroyShaderModule(ctx::device, vertexShaderModule, nullptr);
            vkDestroyShaderModule(ctx::device, fragmentShaderModule, nullptr);

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

            vkDeviceWaitIdle(ctx::device);
            vkDestroyShaderModule(ctx::device, vertexShaderModule, nullptr);
            vkDestroyShaderModule(ctx::device, fragmentShaderModule, nullptr);

            init(lastCreateInfo);

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
            VkCheck(vkCreateShaderModule(ctx::device, &createInfo, nullptr, &shaderModule));
            return shaderModule;
        }
    };
}
