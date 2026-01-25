#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <android/log.h>

extern "C" {
    void __attribute__((constructor)) init() {
        // Força o uso de memória de sistema para buffers de comando (mais rápido no Exynos)
        setenv("VK_ICD_FILENAMES", "/vendor/lib64/hw/vulkan.samsung.so", 1);
        setenv("DISABLE_HW_OVERLAYS", "1", 1);
        setenv("mali_debug_config", "always_clean_caches=true", 1);
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName) {
        return nullptr;
    }

    VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
        *pSupportedVersion = 2;
        return VK_SUCCESS;
    }
}