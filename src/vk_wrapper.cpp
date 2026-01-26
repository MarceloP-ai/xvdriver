#include <vulkan/vulkan.h>
#include <dlfcn.h>
#include <string.h>
#include "imgui/imgui.h"
#include "imgui/imgui_impl_vulkan.h"

typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_vk_icdGetInstanceProcAddr)(VkInstance instance, const char* pName);
static PFN_vk_icdGetInstanceProcAddr real_gippa = nullptr;
static bool imgui_initialized = false;

// Hook para capturar o momento do desenho
VKAPI_ATTR void VKAPI_CALL vkCmdEndRenderPass(VkCommandBuffer commandBuffer) {
    static auto real_end_rp = (PFN_vkCmdEndRenderPass)real_gippa(nullptr, "vkCmdEndRenderPass");
    
    if (imgui_initialized) {
        // Aqui o ImGui desenha seus triângulos no buffer do jogo
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }
    
    real_end_rp(commandBuffer);
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
    // Lógica de frames do ImGui (NewFrame/End)
    if (!imgui_initialized) {
        ImGui::CreateContext();
        imgui_initialized = true;
    }

    ImGui::NewFrame();
    ImGui::Begin("XVD XCLIPSE TUNER");
    ImGui::Text("Device: Samsung S24 (Exynos 2400)");
    ImGui::Text("GPU: Xclipse 940 (RDNA 3)");
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
        if (pName && strcmp(pName, "vkQueuePresentKHR") == 0) return (PFN_vkVoidFunction)vkQueuePresentKHR;
        if (pName && strcmp(pName, "vkCmdEndRenderPass") == 0) return (PFN_vkVoidFunction)vkCmdEndRenderPass;
        if (real_gippa) return real_gippa(instance, pName);
        return nullptr;
    }

    VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
        *pSupportedVersion = 2;
        return VK_SUCCESS;
    }
}