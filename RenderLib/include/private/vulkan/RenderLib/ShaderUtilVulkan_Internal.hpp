#pragma once

#include <RenderLib/ShaderUtil.hpp>
#include <RenderLib/RenderContextVulkan_Internal.hpp>

namespace RenderLib {

    struct VulkanShader : public Shader {

    public:
        std::shared_ptr<VulkanRenderContext> vulkanRenderContext;
        VkShaderModule vkShaderModule;

        VulkanShader(ShaderType shaderType, std::shared_ptr<VulkanRenderContext> renderContext, VkShaderModule vkShaderModule);

        ~VulkanShader() override;
    };

    struct VulkanShaderProgram : public ShaderProgram {

    public:
        std::shared_ptr<VulkanRenderContext> vulkanRenderContext;
        std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;

        explicit VulkanShaderProgram(std::shared_ptr<VulkanRenderContext> vulkanRenderContext, std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos);

        ~VulkanShaderProgram() override;
    };

}
