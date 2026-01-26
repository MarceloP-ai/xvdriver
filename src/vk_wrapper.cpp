#include <vulkan/vulkan.h>
#include <dlfcn.h>
#include <string.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"

// Redefinindo o tipo que o compilador sentiu falta
typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_vk_icdGetInstanceProcAddr)(VkInstance instance, const char* pName);

static PFN_vk_icdGetInstanceProcAddr real_gippa = nullptr;
static bool imgui_initialized = false;

// Hook da criação da tela (Swapchain)
VKAPI_ATTR VkResult VKAPI_CALL vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
    static auto real_create_swapchain = (PFN_vkCreateSwapchainKHR)real_gippa(nullptr, "vkCreateSwapchainKHR");
    imgui_initialized = false; 
    return real_create_swapchain(device, pCreateInfo, pAllocator, pSwapchain);
}

// Hook do frame (Onde o menu aparece)
VKAPI_ATTR VkResult VKAPI_CALL vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
    if (!imgui_initialized) {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        imgui_initialized = true;
    }

    ImGui::NewFrame();
    ImGui::Begin("XVDriver - S24 Turbo");
    ImGui::Text("GPU: Xclipse 940");
    ImGui::Text("Status: Performance Mode Active");
    if(ImGui::Button("Unlock FPS")) {
        // Ação futura
    }
    ImGui::End();
    ImGui::Render();

    static auto real_present = (PFN_vkQueuePresentKHR)real_gippa(nullptr, "vkQueuePresentKHR");
    return real_present(queue, pPresentInfo);
}

extern "C" {
    void __attribute__((constructor)) init() {
        void* handle = dlopen("/vendor/lib64/hw/vulkan.mali.so", RTLD_NOW);
        if (handle) real_gippa = (PFN_vk_icdGetInstanceProcAddr)dlsym(handle, "vk_icdGetInstanceProcAddr");
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName) {
        if (pName && strcmp(pName, "vkCreateSwapchainKHR") == 0) return (PFN_vkVoidFunction)vkCreateSwapchainKHR;
        if (pName && strcmp(pName, "vkQueuePresentKHR") == 0) return (PFN_vkVoidFunction)vkQueuePresentKHR;
        if (real_gippa) return real_gippa(instance, pName);
        return nullptr;
    }

    VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
        *pSupportedVersion = 2;
        return VK_SUCCESS;
    }
}