#pragma once

#include <RenderLib/ShaderUtil.hpp>
#include <RenderLib/ContextManagementVulkan_Internal.hpp>

namespace RenderLib {

    struct VulkanShader : public Shader {

    public:
        VkDevice vkDevice;
        VkShaderModule vkShaderModule;

        VulkanShader(ShaderType shaderType, VkDevice vkDevice, VkShaderModule vkShaderModule);

        ~VulkanShader() override;
    };

    struct VulkanShaderProgram : public ShaderProgram {

    public:
        std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;

        explicit VulkanShaderProgram(std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos);

        ~VulkanShaderProgram() override;
    };

}
