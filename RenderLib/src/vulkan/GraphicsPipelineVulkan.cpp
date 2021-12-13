#include <RenderLib/GraphicsPipelineVulkan_Internal.hpp>
#include <memory>
#include <utility>

namespace RenderLib {

    static inline VkFormat GetVulkanFormat(VertexAttributeType dataType) {
        switch (dataType) {
            case UBYTE:
                return VK_FORMAT_R8_UNORM;
            case BYTE:
                return VK_FORMAT_R8_SNORM;
            case USHORT:
                return VK_FORMAT_R16_UNORM;
            case SHORT:
                return VK_FORMAT_R16_SNORM;
            case INT:
                return VK_FORMAT_R32_SINT;
            case UINT:
                return VK_FORMAT_R32_UINT;
            case FLOAT:
                return VK_FORMAT_R32_SFLOAT;
            case DOUBLE:
                return VK_FORMAT_R64_SFLOAT;
            case MAT2:
                return VK_FORMAT_R32G32_SFLOAT;
            case MAT3:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case MAT4:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            case UBYTE_VEC2:
                return VK_FORMAT_R8G8_UNORM;
            case UBYTE_VEC3:
                return VK_FORMAT_R8G8B8_UNORM;
            case UBYTE_VEC4:
                return VK_FORMAT_R8G8B8A8_UNORM;
            case BYTE_VEC2:
                return VK_FORMAT_R8G8_SNORM;
            case BYTE_VEC3:
                return VK_FORMAT_R8G8B8_SNORM;
            case BYTE_VEC4:
                return VK_FORMAT_R8G8B8A8_SNORM;
            case USHORT_VEC2:
                return VK_FORMAT_R16G16_UNORM;
            case USHORT_VEC3:
                return VK_FORMAT_R16G16B16_UNORM;
            case USHORT_VEC4:
                return VK_FORMAT_R16G16B16A16_UNORM;
            case SHORT_VEC2:
                return VK_FORMAT_R16G16_SNORM;
            case SHORT_VEC3:
                return VK_FORMAT_R16G16B16_SNORM;
            case SHORT_VEC4:
                return VK_FORMAT_R16G16B16A16_SNORM;
            case INT_VEC2:
                return VK_FORMAT_R32G32_SINT;
            case INT_VEC3:
                return VK_FORMAT_R32G32B32_SINT;
            case INT_VEC4:
                return VK_FORMAT_R32G32B32A32_SINT;
            case UINT_VEC2:
                return VK_FORMAT_R32G32_UINT;
            case UINT_VEC3:
                return VK_FORMAT_R32G32B32_UINT;
            case UINT_VEC4:
                return VK_FORMAT_R32G32B32A32_UINT;
            case FLOAT_VEC2:
                return VK_FORMAT_R32G32_SFLOAT;
            case FLOAT_VEC3:
                return VK_FORMAT_R32G32B32_SFLOAT;
            case FLOAT_VEC4:
                return VK_FORMAT_R32G32B32A32_SFLOAT;
            case DOUBLE_VEC2:
                return VK_FORMAT_R64G64_SFLOAT;
            case DOUBLE_VEC3:
                return VK_FORMAT_R64G64B64_SFLOAT;
            case DOUBLE_VEC4:
                return VK_FORMAT_R64G64B64A64_SFLOAT;
            default:
                return VK_FORMAT_UNDEFINED;
        }
    }

    static inline std::vector<VkVertexInputAttributeDescription>
    GetVulkanVertexInputAttributeDescriptions(const VertexFormat &vertexFormat) {
        auto attributes = vertexFormat.getAttributes();
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
        attributeDescriptions.reserve(attributes.size());
        uint32_t binding = 0;
        for (const auto &attribute: attributes) {
            VkVertexInputAttributeDescription attributeDescription{
                    .location = attribute.getLocation(),
                    .binding = binding,
                    .format = GetVulkanFormat(attribute.getDataType()),
                    .offset = static_cast<uint32_t>(attribute.getOffset()),
            };
            attributeDescriptions.push_back(attributeDescription);
            binding++;
        }
        return attributeDescriptions;
    }

    static inline VkVertexInputBindingDescription
    GetVulkanVertexInputBindingDescription(uint32_t binding, const VertexFormat &vertexFormat) {
        VkVertexInputBindingDescription bindingDescription{
                .binding = binding,
                .stride = static_cast<uint32_t>(vertexFormat.getStride()),
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };
        return bindingDescription;
    }

    static inline VkPipelineVertexInputStateCreateInfo GetVertexInputStateCreateInfo(const VertexFormat &vertexFormat,
                                                                                     const std::vector<VkVertexInputBindingDescription> &vertexInputBindingDescriptions,
                                                                                     const std::vector<VkVertexInputAttributeDescription> &vertexAttributeDescriptions) {
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
                .vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindingDescriptions.size()),
                .pVertexBindingDescriptions = vertexInputBindingDescriptions.data(),
                .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescriptions.size()),
                .pVertexAttributeDescriptions = vertexAttributeDescriptions.data()
        };
        return vertexInputInfo;
    }

    static inline VkPipelineInputAssemblyStateCreateInfo
    MakeInputAssemblyStateCreateInfo(VkPrimitiveTopology topology) {
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                .topology = topology,
                .primitiveRestartEnable = VK_FALSE
        };
        return inputAssembly;
    }

    static inline VkViewport MakeViewPort(VkExtent2D viewportExtent) {
        VkViewport viewport{
                .x = 0.0f,
                .y = 0.0f,
                .width = static_cast<float>(viewportExtent.width),
                .height = static_cast<float>(viewportExtent.height),
                .minDepth = 0.0f,
                .maxDepth = 1.0f
        };
        return viewport;
    }

    static inline VkRect2D MakeScissor(VkExtent2D viewportExtent) {
        VkRect2D scissor{
                .offset = {0, 0},
                .extent = viewportExtent
        };
        return scissor;
    }

    static inline VkPipelineViewportStateCreateInfo
    MakeViewportStateCreateInfo(const VkExtent2D &viewportExtent, const VkViewport &viewport,
                                const VkRect2D &scissor) {
        VkPipelineViewportStateCreateInfo viewportState{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
                .viewportCount = 1,
                .pViewports = &viewport,
                .scissorCount = 1,
                .pScissors = &scissor,
        };
        return viewportState;
    }

    static inline VkPipelineRasterizationStateCreateInfo MakeRasterizationInfo() {
        // Expose this shit?
        VkPipelineRasterizationStateCreateInfo rasterizationInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                .depthClampEnable = VK_FALSE,
                .rasterizerDiscardEnable = VK_FALSE,
                .polygonMode = VK_POLYGON_MODE_FILL,
                .frontFace = VK_FRONT_FACE_CLOCKWISE,
                .depthBiasEnable = VK_FALSE,
                .depthBiasConstantFactor = 0.0f,
                .depthBiasClamp = 0.0f,
                .depthBiasSlopeFactor = 0.0f,
                .lineWidth = 1.0f,
        };
        return rasterizationInfo;
    }

    static inline VkPipelineColorBlendStateCreateInfo MakeColorBlendStateCreateInfo() {
        VkPipelineColorBlendAttachmentState colorBlendAttachment{
                .blendEnable = VK_FALSE
        };
        VkPipelineColorBlendStateCreateInfo colorBlendInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
                .logicOpEnable = VK_FALSE,
                .logicOp = VK_LOGIC_OP_COPY,
                .attachmentCount = 1,
                .pAttachments = &colorBlendAttachment
        };
        return colorBlendInfo;
    }

    static inline VkPipelineMultisampleStateCreateInfo MakeMultisampleStateCreateInfo() {
        VkPipelineMultisampleStateCreateInfo multisampleInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
                .sampleShadingEnable = VK_FALSE,
                .minSampleShading = 1.0f,
                .pSampleMask = nullptr,
                .alphaToCoverageEnable = VK_FALSE,
                .alphaToOneEnable = VK_FALSE
        };
        return multisampleInfo;
    }

    static inline VkShaderStageFlags GetShaderTypeFlags(ShaderType type) {
        switch (type) {
            case ShaderType::VERTEX_SHADER:
                return VK_SHADER_STAGE_VERTEX_BIT;
            case ShaderType::FRAGMENT_SHADER:
                return VK_SHADER_STAGE_FRAGMENT_BIT;
            case ShaderType::COMPUTE_SHADER:
                return VK_SHADER_STAGE_COMPUTE_BIT;
            case ShaderType::GEOMETRY_SHADER:
                return VK_SHADER_STAGE_GEOMETRY_BIT;
            default:
                RAISE_EXCEPTION(errorhandling::IllegalStateException, "Invalid shader type");
        }
    }

    /**
     * @param binding the binding number of this entry and corresponds to a resource of the same binding number in the shader stages
     * @param descriptorCount the number of descriptors contained in the binding, accessed in a shader as an array
     * @param acceptingShaderStages the shader stages that the uniform variables are supplied to.
     * @return the descriptor set layout entry
     */
    static inline VkDescriptorSetLayoutBinding
    MakeDescriptorSetLayoutBinding(uint32_t binding, uint32_t descriptorCount,
                                   const std::vector<ShaderType> &acceptingShaderStages) {
        VkShaderStageFlags shaderStageFlags{};
        for (auto shaderStage: acceptingShaderStages) {
            shaderStageFlags |= GetShaderTypeFlags(shaderStage);
        }
        VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{
                .binding = binding,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = descriptorCount,
                .stageFlags = shaderStageFlags,
                .pImmutableSamplers = nullptr
        };
        return descriptorSetLayoutBinding;
    }

    static inline VkDescriptorSetLayout
    CreateDescriptorSetLayout(VkDevice vkDevice, const std::vector<VkDescriptorSetLayoutBinding> &bindings) {
        VkDescriptorSetLayoutCreateInfo layoutInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                .bindingCount = static_cast<uint32_t>(bindings.size()),
                .pBindings = bindings.data()
        };
        VkDescriptorSetLayout descriptorSetLayout;
        VULKAN_STATUS_VALIDATE(vkCreateDescriptorSetLayout(vkDevice, &layoutInfo, nullptr, &descriptorSetLayout),
                               "Failed to create descriptor set layout");
        return descriptorSetLayout;
    }

    static inline VkPipelineLayout CreatePipelineLayout(VkDevice vkDevice, const PipelineLayout &layout) {
        const auto &uniformDescriptors = layout.uniformDescriptors;
        uint32_t binding = 0;
        auto descriptorSetLayouts = std::vector<VkDescriptorSetLayoutBinding>{};
        for (const auto &descriptor: uniformDescriptors) {
            auto elementCount = descriptor.elementCount;
            auto shaderStages = descriptor.acceptingShaderTypes;
            auto descriptorSetLayoutBinding = MakeDescriptorSetLayoutBinding(binding, elementCount, shaderStages);
            descriptorSetLayouts.push_back(descriptorSetLayoutBinding);
            binding++;
        }
        auto descriptorSetLayout = CreateDescriptorSetLayout(vkDevice, descriptorSetLayouts);
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{
                .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                .setLayoutCount = 1,
                .pSetLayouts = &descriptorSetLayout
        };
        VkPipelineLayout pipelineLayout;
        VULKAN_STATUS_VALIDATE(
                vkCreatePipelineLayout(vkDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout),
                "Failed to create pipeline layout"
        );
        vkDestroyDescriptorSetLayout(vkDevice, descriptorSetLayout, nullptr);
        return pipelineLayout;
    }

    static inline std::vector<VkVertexInputBindingDescription>
    GetVulkanVertexInputBindingDescriptions(const VertexFormat &format) {
        auto attributes = format.getAttributes();
        std::vector<VkVertexInputBindingDescription> vertexInputBindingDescriptions{};
        vertexInputBindingDescriptions.reserve(attributes.size());
        uint32_t binding = 0;
        for (const auto &attribute: attributes) {
            vertexInputBindingDescriptions.push_back(GetVulkanVertexInputBindingDescription(binding, format));
            binding++;
        }
        return vertexInputBindingDescriptions;
    }

    VulkanGraphicsPipeline *
    CreateVulkanGraphicsPipelinePtr(const std::shared_ptr<RenderLib::RenderContext> &renderContext,
                                    const VertexFormat &vertexFormat,
                                    const PipelineLayout &pipelineLayout,
                                    const std::shared_ptr<RenderLib::ShaderProgram> &shaderProgram) {
        ENSURE_VULKAN_BACKEND_PTR(renderContext);
        ENSURE_VULKAN_BACKEND_PTR(shaderProgram);

        auto vulkanRenderContext = std::dynamic_pointer_cast<VulkanRenderContext>(renderContext);
        auto vulkanShaderProgram = std::dynamic_pointer_cast<VulkanShaderProgram>(shaderProgram);

        auto shaderStageCreateInfos = vulkanShaderProgram->shaderStageCreateInfos;

        auto inputAssemblyStateCreateInfo = MakeInputAssemblyStateCreateInfo(
                VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST); // TODO: EXPOSE TOPOLOGY
        auto vertexAttributeDescriptions = GetVulkanVertexInputAttributeDescriptions(vertexFormat);
        auto vertexInputBindingDescriptions = GetVulkanVertexInputBindingDescriptions(vertexFormat);
        auto vertexInputInfo = GetVertexInputStateCreateInfo(vertexFormat, vertexInputBindingDescriptions,
                                                             vertexAttributeDescriptions);

        auto viewportExtent = vulkanRenderContext->vulkanSwapChain->swapChainExtent;
        auto viewport = MakeViewPort(viewportExtent);
        auto scissor = MakeScissor(viewportExtent);
        auto viewportStateCreateInfo = MakeViewportStateCreateInfo(viewportExtent, viewport, scissor);
        auto rasterizationStateCreateInfo = MakeRasterizationInfo();
        auto colorBlendStateCreateInfo = MakeColorBlendStateCreateInfo();
        auto multisampleStateCreateInfo = MakeMultisampleStateCreateInfo();
        auto vkPipelineLayout = CreatePipelineLayout(vulkanRenderContext->vkDevice, pipelineLayout);

        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
                .stageCount = static_cast<uint32_t>(shaderStageCreateInfos.size()),
                .pStages = shaderStageCreateInfos.data(),
                .pVertexInputState = &vertexInputInfo,
                .pInputAssemblyState = &inputAssemblyStateCreateInfo,
                .pViewportState = &viewportStateCreateInfo,
                .pRasterizationState = &rasterizationStateCreateInfo,
                .pMultisampleState = &multisampleStateCreateInfo,
                .pDepthStencilState = nullptr,
                .pColorBlendState = &colorBlendStateCreateInfo,
                .pDynamicState = nullptr,
                .layout = vkPipelineLayout,
                .renderPass = vulkanRenderContext->vkRenderPass,
                .subpass = 0,
                .basePipelineHandle = VK_NULL_HANDLE,
                .basePipelineIndex = -1
        };

        VkPipeline vkPipeline;

        VULKAN_STATUS_VALIDATE(
                vkCreateGraphicsPipelines(
                        vulkanRenderContext->vkDevice,
                        nullptr,
                        1,
                        &pipelineCreateInfo,
                        nullptr,
                        &vkPipeline
                ),
                "Failed to create graphics pipeline."
        );
        auto graphicsPipeline = new VulkanGraphicsPipeline(
                vertexFormat, pipelineLayout, shaderProgram, vulkanRenderContext, vkPipeline, vkPipelineLayout
        );
        return graphicsPipeline;
    }

    std::shared_ptr<GraphicsPipeline>
    CreateGraphicsPipeline(const std::shared_ptr<RenderLib::RenderContext> &renderContext,
                           const VertexFormat &vertexFormat,
                           const PipelineLayout &pipelineLayout,
                           const std::shared_ptr<RenderLib::ShaderProgram> &shaderProgram) {
        ENSURE_VULKAN_BACKEND_PTR(renderContext);
        ENSURE_VULKAN_BACKEND_PTR(shaderProgram);

        auto vulkanRenderContext = std::dynamic_pointer_cast<VulkanRenderContext>(renderContext);
        auto graphicsPipeline = std::shared_ptr<VulkanGraphicsPipeline>(CreateVulkanGraphicsPipelinePtr(vulkanRenderContext, vertexFormat, pipelineLayout,
                                                             shaderProgram));
        vulkanRenderContext->vulkanGraphicsPipelines.push_back(graphicsPipeline);
        return graphicsPipeline;
    }

    VulkanGraphicsPipeline::VulkanGraphicsPipeline(const VertexFormat &vertexFormat,
                                                   const PipelineLayout &pipelineLayout,
                                                   std::shared_ptr<ShaderProgram> shaderProgram,
                                                   std::shared_ptr<VulkanRenderContext> vulkanRenderContext,
                                                   VkPipeline vkPipeline, VkPipelineLayout vkPipelineLayout)
            : GraphicsPipeline(VULKAN, vertexFormat, pipelineLayout, std::move(shaderProgram)),
              vulkanRenderContext(std::move(vulkanRenderContext)),
              vkPipeline(vkPipeline),
              vkPipelineLayout(vkPipelineLayout) {
    }

    void VulkanGraphicsPipeline::dispose() {
        if (disposed) {
            return;
        }
        vkDestroyPipeline(vulkanRenderContext->vkDevice, vkPipeline, nullptr);
        vkDestroyPipelineLayout(vulkanRenderContext->vkDevice, vkPipelineLayout, nullptr);
        vulkanRenderContext = nullptr;
        disposed = true;
    }

    void VulkanGraphicsPipeline::transferStateFrom(VulkanGraphicsPipeline *graphicsPipeline) {
        vkPipeline = graphicsPipeline->vkPipeline;
        vkPipelineLayout = graphicsPipeline->vkPipelineLayout;
        pipelineLayout = graphicsPipeline->pipelineLayout;
        shaderProgram = graphicsPipeline->shaderProgram;
        vertexFormat = graphicsPipeline->vertexFormat;
        vulkanRenderContext = graphicsPipeline->vulkanRenderContext;
        graphicsPipeline->disposed = true; // Hack to prevent freeing the handles we just transferred
    }

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline() {
        dispose();
    }

}