#include <RenderLib/BufferObjectVulkan_Internal.hpp>
#include <ErrorHandling/IllegalStateException.hpp>

namespace RenderLib {

    static VkBufferUsageFlags GetBufferUsageBits(BufferType bufferType) {
        switch (bufferType) {
            case BufferType::VERTEX_BUFFER:
                return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
            case BufferType::INDEX_BUFFER:
                return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
            default:
                RAISE_EXCEPTION(errorhandling::IllegalStateException, "Unknown buffer type");
        }
    }

    static uint32_t
    FindMemoryType(VkPhysicalDevice vkPhysicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        RAISE_EXCEPTION(errorhandling::IllegalStateException, "Failed to find suitable memory type");
    }

    std::shared_ptr<BufferObject> CreateBufferObject(const std::shared_ptr<RenderLib::RenderContext> &renderContext,
                                                     const BufferDescriptor &bufferDescriptor,
                                                     const void *data, size_t dataSize) {
        ENSURE_VULKAN_BACKEND_PTR(renderContext);
        auto vulkanRenderContext = std::dynamic_pointer_cast<RenderLib::VulkanRenderContext>(renderContext);
        VkBufferCreateInfo bufferCreateInfo = {
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = bufferDescriptor.bufferSize,
                .usage = GetBufferUsageBits(bufferDescriptor.bufferType),
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE
        };

        VkBuffer vkBuffer;
        VULKAN_STATUS_VALIDATE(vkCreateBuffer(vulkanRenderContext->vkDevice, &bufferCreateInfo, nullptr, &vkBuffer),
                               "Failed to create buffer");

        VkMemoryRequirements memoryRequirements{};
        vkGetBufferMemoryRequirements(vulkanRenderContext->vkDevice, vkBuffer, &memoryRequirements);

        VkPhysicalDeviceMemoryProperties memoryProperties{};
        vkGetPhysicalDeviceMemoryProperties(vulkanRenderContext->vkPhysicalDevice, &memoryProperties);

        VkMemoryAllocateInfo memoryAllocateInfo = {
                .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
                .allocationSize = memoryRequirements.size,
                .memoryTypeIndex =
                FindMemoryType(
                        vulkanRenderContext->vkPhysicalDevice,
                        memoryRequirements.memoryTypeBits,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                )
        };

        VkDeviceMemory vkDeviceMemory;
        VULKAN_STATUS_VALIDATE(
                vkAllocateMemory(vulkanRenderContext->vkDevice, &memoryAllocateInfo, nullptr, &vkDeviceMemory),
                "Failed to allocate memory"
        );

        VULKAN_STATUS_VALIDATE(vkBindBufferMemory(vulkanRenderContext->vkDevice, vkBuffer, vkDeviceMemory, 0),
                               "Failed to bind buffer memory");

        void *mappedMemory;
        VULKAN_STATUS_VALIDATE(
                vkMapMemory(vulkanRenderContext->vkDevice, vkDeviceMemory, 0, bufferCreateInfo.size, 0, &mappedMemory),
                "Failed to map memory"
        );

        if (data != nullptr) {
            memcpy(mappedMemory, data, dataSize);
        }

        vkUnmapMemory(vulkanRenderContext->vkDevice, vkDeviceMemory);

        return std::make_shared<VulkanBufferObject>(vulkanRenderContext->vkDevice, vkBuffer,
                                                    bufferDescriptor.bufferType);
    }

    VulkanBufferObject::~VulkanBufferObject() {
        vkDestroyBuffer(vkDevice, vkBuffer, nullptr);
    }

    VulkanBufferObject::VulkanBufferObject(VkDevice vkDevice, VkBuffer vkBuffer, BufferType bufferType) :

            BufferObject(VULKAN, bufferType),
            vkDevice(vkDevice),
            vkBuffer(vkBuffer) {
    }

}