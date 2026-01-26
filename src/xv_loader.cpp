#include "xv_loader.h"
#include "vk_dispatch_table.h"
#include <vulkan/vulkan.h>
#include <mutex>

static std::once_flag initFlag;

// Ponteiro para vkGetInstanceProcAddr original
PFN_vkGetInstanceProcAddr real_vkGetInstanceProcAddr = nullptr;

// Dispatch table
InstanceDispatchTable instanceDispatch = {0};

static void loadRealVulkan() {
    real_vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr) 
        vkGetInstanceProcAddr(nullptr, "vkGetInstanceProcAddr");
}

// O nosso interceptor principal
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xvGetInstanceProcAddr(
        VkInstance instance,
        const char* pName) {

    std::call_once(initFlag, loadRealVulkan);

    // Se for pedido nosso interceptor → devolve
    if (strcmp(pName, "vkGetInstanceProcAddr") == 0) {
        return (PFN_vkVoidFunction)xvGetInstanceProcAddr;
    }
    if (strcmp(pName, "vkGetDeviceProcAddr") == 0) {
        return (PFN_vkVoidFunction)xvGetDeviceProcAddr;
    }

    // Intercepte aqui chamadas específicas
    if (strcmp(pName, "vkCreateInstance") == 0) {
        return (PFN_vkVoidFunction)real_vkGetInstanceProcAddr(instance, "vkCreateInstance");
    }

    // Senão, use a função real
    return real_vkGetInstanceProcAddr(instance, pName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xvGetDeviceProcAddr(
        VkDevice device,
        const char* pName) {

    if (instanceDispatch.vkGetDeviceProcAddr) {
        return instanceDispatch.vkGetDeviceProcAddr(device, pName);
    }
    return nullptr;
}
