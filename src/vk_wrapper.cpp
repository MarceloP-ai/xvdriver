#include <vulkan/vulkan.h>
#include <dlfcn.h>
#include <string.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"

typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_vk_icdGetInstanceProcAddr)(VkInstance instance, const char* pName);
static PFN_vk_icdGetInstanceProcAddr real_gippa = nullptr;
static bool imgui_initialized = false;
static VkDescriptorPool g_DescriptorPool = VK_NULL_HANDLE;

// Função auxiliar para criar o pool de memória do Menu
void CreateDescriptorPool(VkDevice device) {
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(device, &pool_info, nullptr, &g_DescriptorPool);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
    if (!imgui_initialized) {
        ImGui::CreateContext();
        // Aqui no futuro adicionaremos o ImGui_ImplVulkan_Init
        imgui_initialized = true;
    }

    ImGui::NewFrame();
    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin("XVDriver - S24 ULTRA TURBO");
    ImGui::Text("Interface: Vulkan Bypass");
    ImGui::Separator();
    if(ImGui::Button("MAX PERFORMANCE")) {
        // Trigger de performance
    }
    ImGui::End();
    ImGui::Render();

    // O RenderDrawData será enviado para o CommandBuffer aqui na v21
    
    static auto real_present = (PFN_vkQueuePresentKHR)real_gippa(nullptr, "vkQueuePresentKHR");
    return real_present(queue, pPresentInfo);
}

extern "C" {
    void __attribute__((constructor)) init() {
        void* handle = dlopen("/vendor/lib64/hw/vulkan.mali.so", RTLD_NOW);
        if (handle) real_gippa = (PFN_vk_icdGetInstanceProcAddr)dlsym(handle, "vk_icdGetInstanceProcAddr");
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName) {
        if (pName && strcmp(pName, "vkQueuePresentKHR") == 0) return (PFN_vkVoidFunction)vkQueuePresentKHR;
        if (real_gippa) return real_gippa(instance, pName);
        return nullptr;
    }

    VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
        *pSupportedVersion = 2;
        return VK_SUCCESS;
    }
}