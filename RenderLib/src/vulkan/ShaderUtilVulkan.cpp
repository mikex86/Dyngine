#include <RenderLib/ShaderUtilVulkan_Internal.hpp>
#include <memory>
#include <utility>

namespace RenderLib {

    std::shared_ptr<RenderLib::Shader>
    LoadPrecompiledShader(const std::shared_ptr<RenderLib::RenderContext> &renderContext,
                          const uint8_t *shaderBinaryContent,
                          size_t shaderBinaryContentSize, ShaderType shaderType) {
        ENSURE_VULKAN_BACKEND_PTR(renderContext);
        auto vulkanRenderContext = std::dynamic_pointer_cast<VulkanRenderContext>(renderContext);
        VkShaderModule vkShaderModule;
        {
            VkShaderModuleCreateInfo createInfo{
                    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                    .codeSize = shaderBinaryContentSize,
                    .pCode = reinterpret_cast<const uint32_t *>(shaderBinaryContent),
            };
            VULKAN_STATUS_VALIDATE(
                    vkCreateShaderModule(vulkanRenderContext->vkDevice, &createInfo, nullptr, &vkShaderModule),
                    "Failed to create shader module"
            );
        }
        return std::shared_ptr<RenderLib::Shader>(new VulkanShader(shaderType, vulkanRenderContext, vkShaderModule));
    }

    static VkShaderStageFlagBits GetVulkanShaderStageBits(ShaderType shaderType) {
        switch (shaderType) {
            case ShaderType::VERTEX_SHADER:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderType::FRAGMENT_SHADER:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            case ShaderType::COMPUTE_SHADER:
                return VK_SHADER_STAGE_COMPUTE_BIT;
            case ShaderType::GEOMETRY_SHADER:
                return VK_SHADER_STAGE_GEOMETRY_BIT;
            default:
                RAISE_EXCEPTION(errorhandling::IllegalStateException, "Unknown shader type");
        }
    }

    std::shared_ptr<RenderLib::ShaderProgram>
    CreateShaderProgram(const std::shared_ptr<RenderLib::RenderContext> &renderContext,
                        const std::map<ShaderType, Shader *> &shaders) {
        ENSURE_VULKAN_BACKEND_PTR(renderContext);
        auto vulkanRenderContext = std::dynamic_pointer_cast<VulkanRenderContext>(renderContext);
        std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos;
        for (const auto &item: shaders) {
            auto shaderType = item.first;
            auto shader = item.second;
            ENSURE_VULKAN_BACKEND_PTR(shader);
            auto vulkanShader = reinterpret_cast<VulkanShader *>(shader);
            if (vulkanShader->shaderType != shaderType) {
                RAISE_EXCEPTION(errorhandling::IllegalStateException, "Shader type mismatch. Shader is of type "
                                                                      + GetShaderTypeName(vulkanShader->shaderType) +
                                                                      " but "
                                                                      + GetShaderTypeName(shaderType) +
                                                                      " was set in map.");
            }
            VkPipelineShaderStageCreateInfo shaderStageInfo{
                    .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                    .stage = GetVulkanShaderStageBits(shaderType),
                    .module = vulkanShader->vkShaderModule,
                    .pName = "main"
            };
            shaderStageCreateInfos.push_back(shaderStageInfo);
        }
        return std::make_shared<VulkanShaderProgram>(vulkanRenderContext, shaderStageCreateInfos);
    }

    VulkanShader::VulkanShader(ShaderType shaderType, std::shared_ptr<VulkanRenderContext> vulkanRenderContext,
                               VkShaderModule vkShaderModule) :
            Shader(VULKAN, shaderType),
            vulkanRenderContext(std::move(vulkanRenderContext)),
            vkShaderModule(vkShaderModule) {
    }

    VulkanShader::~VulkanShader() {
        vkDestroyShaderModule(vulkanRenderContext->vkDevice, vkShaderModule, nullptr);
    }

    VulkanShaderProgram::VulkanShaderProgram(std::shared_ptr<VulkanRenderContext> vulkanRenderContext,
                                             std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos)
            : ShaderProgram(VULKAN),
              vulkanRenderContext(std::move(vulkanRenderContext)),
              shaderStageCreateInfos(std::move(shaderStageCreateInfos)) {
    }

    VulkanShaderProgram::~VulkanShaderProgram() = default;
}