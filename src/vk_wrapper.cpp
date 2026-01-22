#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <iostream>
#include <cstring>

// Função de apoio (hook)
VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateInstance(const VkInstanceCreateInfo* pCI, const VkAllocationCallbacks* pA, VkInstance* pI) {
    std::cout << "\n\n[XVDriver] SUCESSO: LAYER INJETADA!\n\n" << std::endl;
    VkLayerInstanceCreateInfo* ci = (VkLayerInstanceCreateInfo*)pCI->pNext;
    while (ci && !(ci->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO && ci->function == VK_LAYER_LINK_INFO)) {
        ci = (VkLayerInstanceCreateInfo*)ci->pNext;
    }
    if (!ci) return VK_ERROR_INITIALIZATION_FAILED;
    PFN_vkGetInstanceProcAddr nextGipa = ci->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    ci->u.pLayerInfo = ci->u.pLayerInfo->pNext;
    PFN_vkCreateInstance nextCreate = (PFN_vkCreateInstance)nextGipa(VK_NULL_HANDLE, "vkCreateInstance");
    return nextCreate(pCI, pA, pI);
}

// Funções padrão da Layer
extern "C" {
    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance inst, const char* name) {
        if (strcmp(name, "vkCreateInstance") == 0) return (PFN_vkVoidFunction)xv_vkCreateInstance;
        return nullptr;
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice dev, const char* name) {
        return nullptr;
    }
}