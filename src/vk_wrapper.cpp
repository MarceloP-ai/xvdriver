#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <string.h>
#include <android/log.h>

#define LOG_TAG "XVD_DEBUG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" {

    // Nome oficial que o Android busca para Layers
    VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
        LOGI("XVD_DEBUG: Success! Hooked into Vulkan System.");
        return VK_SUCCESS; 
    }

    // Função de entrada obrigatória para Layers de Sistema
    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
        if (strcmp(pName, "vkEnumerateDeviceExtensionProperties") == 0) {
            return (PFN_vkVoidFunction)vkEnumerateDeviceExtensionProperties;
        }
        return nullptr;
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* pName) {
        return nullptr;
    }
    
    // Exportação obrigatória para drivers Android
    VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct) {
        LOGI("XVD_DEBUG: Loader handshake initiated.");
        pVersionStruct->pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = vkGetDeviceProcAddr;
        return VK_SUCCESS;
    }
}