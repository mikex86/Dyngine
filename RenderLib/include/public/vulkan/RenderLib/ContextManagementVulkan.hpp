#pragma once

#include <RenderLib/ContextManagement.hpp>
#include <ErrorHandling/ErrorHandling.hpp>
#include <string>
#include <vector>

namespace RenderLib {

    struct VulkanApplication {
        std::string applicationName;
        uint32_t applicationVersionMajor;
        uint32_t applicationVersionMinor;
        uint32_t applicationVersionPatch;
        std::string engineName;
        uint32_t engineVersionMajor;
        uint32_t engineVersionMinor;
        uint32_t engineVersionPatch;
    };

    struct VulkanRenderSystemConfig : public RenderSystemConfig {
        VulkanApplication application;
        std::vector<std::string> instanceExtensions;
        std::vector<std::string> instanceLayers;
    };

    struct VulkanRenderSystem;

    NEW_EXCEPTION_TYPE(VulkanStatusException);

}