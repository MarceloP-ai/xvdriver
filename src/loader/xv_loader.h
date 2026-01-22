#pragma once

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xvGetInstanceProcAddr(
        VkInstance instance,
        const char* pName);

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xvGetDeviceProcAddr(
        VkDevice device,
        const char* pName);

#ifdef __cplusplus
}
#endif