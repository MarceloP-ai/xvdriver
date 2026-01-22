#pragma once
#include <vulkan/vulkan.h>

struct InstanceDispatchTable {
    PFN_vkGetInstanceProcAddr GetInstanceProcAddr;
    PFN_vkGetPhysicalDeviceMemoryProperties GetPhysicalDeviceMemoryProperties;
};

void xv_init_instance_dispatch(VkInstance instance, PFN_vkGetInstanceProcAddr gipa);
InstanceDispatchTable* xv_get_instance_dispatch(VkInstance instance);
