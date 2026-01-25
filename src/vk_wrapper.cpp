#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <string.h>
#include <android/log.h>

#define LOG_TAG "XVD_DEBUG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

// Função global para interceptar extensões
extern "C" {
    
    VKAPI_ATTR VkResult VKAPI_CALL xv_EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
        LOGI("XVD_DEBUG: Intercepted EnumerateDeviceExtensionProperties");
        
        // Chamada original aqui seria complexa sem o loader, então vamos apenas logar que chegamos aqui
        if (pProperties) {
            LOGI("XVD_DEBUG: Eden is querying extensions now!");
        }
        
        return VK_SUCCESS; 
    }

    // O loader do Android procura por esta função específica
    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
        LOGI("XVD_DEBUG: GetInstanceProcAddr for -> %s", pName);
        if (strcmp(pName, "vkEnumerateDeviceExtensionProperties") == 0) return (PFN_vkVoidFunction)xv_EnumerateDeviceExtensionProperties;
        return nullptr;
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* pName) {
        return nullptr;
    }
}