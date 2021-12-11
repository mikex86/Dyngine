#pragma once

#include <RenderLib/BufferObject.hpp>
#include <RenderLib/RenderLibVulkan_Internal.hpp>
#include <RenderLib/RenderContextVulkan_Internal.hpp>
#include <vulkan/vulkan.h>

namespace RenderLib {

    struct VulkanBufferObject : public BufferObject {

        VkDevice vkDevice;
        VkBuffer vkBuffer;

        VulkanBufferObject(VkDevice vkDevice, VkBuffer vkBuffer, BufferType bufferType);

        virtual ~VulkanBufferObject();

    };

}