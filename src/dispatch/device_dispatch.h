#pragma once
#include <vulkan/vulkan.h>

struct DeviceDispatchTable {
    PFN_vkGetDeviceProcAddr GetDeviceProcAddr;
    PFN_vkCreateSwapchainKHR CreateSwapchainKHR;
    PFN_vkGetSwapchainImagesKHR GetSwapchainImagesKHR;
    PFN_vkCreateBuffer CreateBuffer;
    PFN_vkGetBufferMemoryRequirements GetBufferMemoryRequirements;
    PFN_vkAllocateMemory AllocateMemory;
    PFN_vkBindBufferMemory BindBufferMemory;
    PFN_vkMapMemory MapMemory;
    PFN_vkCreateCommandPool CreateCommandPool;
    PFN_vkAllocateCommandBuffers AllocateCommandBuffers;
    PFN_vkBeginCommandBuffer BeginCommandBuffer;
    PFN_vkEndCommandBuffer EndCommandBuffer;
    PFN_vkCmdPipelineBarrier CmdPipelineBarrier;
    PFN_vkCmdCopyBufferToImage CmdCopyBufferToImage;
    PFN_vkQueueSubmit QueueSubmit;
    PFN_vkQueuePresentKHR QueuePresentKHR;
    PFN_vkResetCommandPool ResetCommandPool;
    PFN_vkQueueWaitIdle QueueWaitIdle;
};

void xv_init_device_dispatch(VkDevice device, PFN_vkGetDeviceProcAddr gdpa);
DeviceDispatchTable* xv_get_device_dispatch(VkDevice device);
void xv_set_queue_mapping(VkQueue queue, VkDevice device);
DeviceDispatchTable* xv_get_queue_dispatch(VkQueue queue);
