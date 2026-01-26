#include <vulkan/vulkan.h>
#include <dlfcn.h>
#include <string.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"

typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_vk_icdGetInstanceProcAddr)(VkInstance instance, const char* pName);
static PFN_vk_icdGetInstanceProcAddr real_gippa = nullptr;

// Variáveis globais para armazenar o estado da GPU
static VkDevice g_Device = VK_NULL_HANDLE;
static VkPhysicalDevice g_PhysDevice = VK_NULL_HANDLE;
static VkQueue g_Queue = VK_NULL_HANDLE;
static bool imgui_initialized = false;

// Hook para capturar o Dispositivo e a GPU Física
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
    static auto real_create_device = (PFN_vkCreateDevice)real_gippa(nullptr, "vkCreateDevice");
    g_PhysDevice = physicalDevice;
    VkResult res = real_create_device(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (res == VK_SUCCESS) g_Device = *pDevice;
    return res;
}

// Hook para capturar a Fila de Processamento (Queue)
VKAPI_ATTR void VKAPI_CALL vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
    static auto real_get_queue = (PFN_vkGetDeviceQueue)real_gippa(nullptr, "vkGetDeviceQueue");
    real_get_queue(device, queueFamilyIndex, queueIndex, pQueue);
    if (pQueue) g_Queue = *pQueue;
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
    if (g_Device != VK_NULL_HANDLE && !imgui_initialized) {
        ImGui::CreateContext();
        // O Init completo do Backend Vulkan virá aqui após validarmos esses ponteiros
        imgui_initialized = true;
    }

    if (imgui_initialized) {
        ImGui::NewFrame();
        ImGui::Begin("XVD TEST - S24");
        ImGui::Text("Ponteiros capturados!");
        ImGui::Text("Device: %p", (void*)g_Device);
        ImGui::Text("Queue: %p", (void*)g_Queue);
        ImGui::End();
        ImGui::Render();
    }

    static auto real_present = (PFN_vkQueuePresentKHR)real_gippa(nullptr, "vkQueuePresentKHR");
    return real_present(queue, pPresentInfo);
}

extern "C" {
    void __attribute__((constructor)) init() {
        void* handle = dlopen("/vendor/lib64/hw/vulkan.mali.so", RTLD_NOW);
        if (handle) real_gippa = (PFN_vk_icdGetInstanceProcAddr)dlsym(handle, "vk_icdGetInstanceProcAddr");
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName) {
        if (pName && strcmp(pName, "vkCreateDevice") == 0) return (PFN_vkVoidFunction)vkCreateDevice;
        if (pName && strcmp(pName, "vkGetDeviceQueue") == 0) return (PFN_vkVoidFunction)vkGetDeviceQueue;
        if (pName && strcmp(pName, "vkQueuePresentKHR") == 0) return (PFN_vkVoidFunction)vkQueuePresentKHR;
        if (real_gippa) return real_gippa(instance, pName);
        return nullptr;
    }

    VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
        *pSupportedVersion = 2;
        return VK_SUCCESS;
    }
}