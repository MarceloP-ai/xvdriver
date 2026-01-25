#include <vulkan/vulkan.h>
#include <android/log.h>
#include <string.h>

#define LOG_TAG "XVD_DEBUG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" {
    // Intercepta a listagem de extensões para esconder o que o ANGLE quer
    VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
        LOGI("XVD_DEBUG: ANGLE is checking extensions. Filtering for performance...");
        
        // Aqui nós vamos "capar" extensões de sincronização pesadas no futuro
        return VK_SUCCESS;
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName) {
        LOGI("XVD_DEBUG: Function Hooked -> %s", pName);
        if (strcmp(pName, "vkEnumerateDeviceExtensionProperties") == 0) 
            return (PFN_vkVoidFunction)vkEnumerateDeviceExtensionProperties;
        return nullptr;
    }

    VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
        LOGI("XVD_DEBUG: ICD Handshake Success");
        *pSupportedVersion = 2;
        return VK_SUCCESS;
    }
}