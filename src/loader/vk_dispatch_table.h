#pragma once
#include <vulkan/vulkan.h>

extern PFN_vkGetInstanceProcAddr real_vkGetInstanceProcAddr;

// Dispatch table para instance
typedef struct {
    PFN_vkDestroyInstance vkDestroyInstance;
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
    PFN_vkCreateDevice vkCreateDevice;
} InstanceDispatchTable;

extern InstanceDispatchTable instanceDispatch;