#include <chrono>
#include <iostream>

// Funcao de calculo de FPS injetada
void UpdateFPS() {
    static auto lastTime = std::chrono::high_resolution_clock::now();
    static uint32_t frameCount = 0;
    frameCount++;
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = currentTime - lastTime;
    if (elapsed.count() >= 1.0) {
        std::cout << "[XVDriver] FPS: " << (frameCount / elapsed.count()) << std::endl;
        frameCount = 0;
        lastTime = currentTime;
    }
}
#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <iostream>
#include <cstring>

// FunÃ§Ã£o de apoio (hook)
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

// FunÃ§Ãµes padrÃ£o da Layer
extern "C" {
    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance inst, const char* name) {
        if (strcmp(name, "vkCreateInstance") == 0) return (PFN_vkVoidFunction)xv_vkCreateInstance;
        return nullptr;
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice dev, const char* name) {
        return nullptr;
    }
}

extern "C" VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct) {
    if (pVersionStruct->loader_layer_interface_version >= 2) {
        pVersionStruct->loader_layer_interface_version = 2;
        pVersionStruct->pfnGetInstanceProcAddr = vkGetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = vkGetDeviceProcAddr;
        pVersionStruct->pfnGetPhysicalDeviceProcAddr = nullptr;
    }
    return VK_SUCCESS;
}
