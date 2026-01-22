#include <vulkan/vulkan.h>
#include <cstring>

void xv_mask_device_features(
    VkDeviceCreateInfo& createInfo,
    VkPhysicalDeviceFeatures& featuresOut) {

    if (createInfo.pEnabledFeatures) {
        featuresOut = *createInfo.pEnabledFeatures;
    } else {
        std::memset(&featuresOut, 0, sizeof(featuresOut));
    }

    /* ---------------------------------------------------------- */
    /*  SAFE FEATURES (DXVK / EMULATORS FRIENDLY)                 */
    /* ---------------------------------------------------------- */

    featuresOut.samplerAnisotropy = VK_TRUE;
    featuresOut.shaderClipDistance = VK_TRUE;
    featuresOut.shaderCullDistance = VK_TRUE;
    featuresOut.textureCompressionBC = VK_TRUE;

    /* ⚠️ NÃO force geometry/tessellation (crash em mobile) */
    featuresOut.geometryShader = VK_FALSE;
    featuresOut.tessellationShader = VK_FALSE;

    createInfo.pEnabledFeatures = &featuresOut;
}
