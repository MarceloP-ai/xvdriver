#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>
#include <android/log.h>
#include <string.h>

#define LOG_TAG "XVD_EDEN"
#define EXPORT __attribute__((visibility("default")))

// Definição manual necessária para compilar em alguns NDKs
typedef enum VkNegotiateLayerStructType {
    LAYER_NEGOTIATE_UNINTIALIZED = 0,
    LAYER_NEGOTIATE_INTERFACE_STRUCT = 1,
} VkNegotiateLayerStructType;

typedef struct VkNegotiateLayerInterface {
    VkNegotiateLayerStructType sType;
    void* pNext;
    uint32_t loaderLayerInterfaceVersion;
    PFN_vkGetInstanceProcAddr pfnGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr pfnGetDeviceProcAddr;
    PFN_vkVoidFunction pfnGetPhysicalDeviceProcAddr;
} VkNegotiateLayerInterface;

extern "C" {
    // Agora o compilador saberá o que é VkNegotiateLayerInterface
    EXPORT VkResult vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct) {
        if (pVersionStruct->loaderLayerInterfaceVersion < 2) return VK_ERROR_INITIALIZATION_FAILED;
        pVersionStruct->loaderLayerInterfaceVersion = 2;
        return VK_SUCCESS;
    }

    EXPORT PFN_vkVoidFunction vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
        return nullptr; 
    }
}
