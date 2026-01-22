#pragma once
#include <vulkan/vulkan.h>

#ifdef __cplusplus
extern "C" {
#endif

void xv_init_command_buffer_dispatch(
    VkCommandBuffer commandBuffer,
    PFN_vkGetDeviceProcAddr gdpa
);

#ifdef __cplusplus
}
#endif
