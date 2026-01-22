#pragma once

#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

// Inicializa o wrapper e carrega o driver Vulkan real
void xv_init_vulkan_loader(void);

// Interceptador principal
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL
xv_vkGetInstanceProcAddr(VkInstance instance, const char* pName);

// Ponteiro real (driver do sistema)
extern PFN_vkGetInstanceProcAddr real_vkGetInstanceProcAddr;

#ifdef __cplusplus
}
#endif