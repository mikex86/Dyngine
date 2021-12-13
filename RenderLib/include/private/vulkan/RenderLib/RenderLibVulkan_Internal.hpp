#pragma once

#define RENDERLIB_DEBUG_VALIDATIONS

#ifdef RENDERLIB_DEBUG_VALIDATIONS
#define ENSURE_VULKAN_BACKEND_PTR(handle) \
    if ((handle)->backend != RenderSystemBackend::VULKAN) \
    RAISE_EXCEPTION(errorhandling::IllegalStateException, \
                    std::string("Handle "  #handle  " has an unexpected backend: ") + \
                    GetRenderSystemBackendName((handle)->backend) + ", should be Vulkan instead!")

#define ENSURE_VULKAN_BACKEND(handle) \
    if ((handle).backend != RenderSystemBackend::VULKAN) \
    RAISE_EXCEPTION(errorhandling::IllegalStateException, \
                    std::string("Handle " #handle " has an unexpected backend: ") + \
                    GetRenderSystemBackendName((handle).backend) + ", should be Vulkan instead!")
#else
#define ENSURE_VULKAN_BACKEND_PTR(renderSystem)
#define ENSURE_VULKAN_BACKEND(renderSystem)
#endif

#ifdef RENDERLIB_DEBUG_VALIDATIONS
#define VULKAN_STATUS_VALIDATE_WITH_CALLBACK(errorStatus, errorMessage, errorCallback) \
{                                                                                      \
auto status = (errorStatus);                                                                                       \
if ((status) != VK_SUCCESS) {                                       \
    errorCallback;                                                                     \
    RAISE_EXCEPTION(VulkanStatusException, std::string(errorMessage) + ": " + RenderLib::GetVulkanResultString(status)); \
} \
}
#else
#define VULKAN_STATUS_VALIDATE_WITH_CALLBACK(errorStatus, errorMessage, errorCallback) if ((errorStatus) != VK_SUCCESS) { errorCallBack;}
#endif

#define VULKAN_STATUS_VALIDATE(errorStatus, errorMessage) VULKAN_STATUS_VALIDATE_WITH_CALLBACK(errorStatus, errorMessage, {})
