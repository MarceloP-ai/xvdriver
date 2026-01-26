#pragma once

#include <vulkan/vulkan.h>

void xv_mask_device_features(
    VkDeviceCreateInfo& createInfo,
    VkPhysicalDeviceFeatures& featuresOut);
