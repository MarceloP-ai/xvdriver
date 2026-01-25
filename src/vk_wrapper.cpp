#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_win32.h>
#include <string>
#include <chrono>

struct OverlayContext {
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    bool isInitialized = false;
    float fps = 0.0f;
};

OverlayContext g_Overlay;
PFN_vkQueuePresentKHR g_pfnNextQueuePresent = nullptr;
PFN_vkCreateSwapchainKHR g_pfnNextCreateSwapchain = nullptr;
PFN_vkCreateRenderPass g_pfnNextCreateRenderPass = nullptr;
PFN_vkEndCommandBuffer g_pfnNextEndCommandBuffer = nullptr;

void UpdateFPS() {
    static auto lastTime = std::chrono::high_resolution_clock::now();
    static int frames = 0;
    auto currentTime = std::chrono::high_resolution_clock::now();
    frames++;
    std::chrono::duration<float> elapsed = currentTime - lastTime;
    if (elapsed.count() >= 1.0f) {
        g_Overlay.fps = frames / elapsed.count();
        frames = 0;
        lastTime = currentTime;
    }
}

extern "C" {
    // Hook para injetar o desenho no final da lista de comandos
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkEndCommandBuffer(VkCommandBuffer commandBuffer) {
        if (g_Overlay.isInitialized) {
            ImGui_ImplVulkan_NewFrame();
            ImGui::NewFrame();
            
            // Janela do FPS
            ImGui::SetNextWindowPos(ImVec2(10, 10));
            ImGui::Begin("XVDriver Overlay", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoSavedSettings);
            ImGui::Text("FPS: %.1f", g_Overlay.fps);
            ImGui::End();
            
            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
        }
        return g_pfnNextEndCommandBuffer(commandBuffer);
    }

    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
        VkResult result = g_pfnNextCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
        if (result == VK_SUCCESS) g_Overlay.renderPass = *pRenderPass;
        return result;
    }

    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
        g_Overlay.device = device;
        return g_pfnNextCreateSwapchain(device, pCreateInfo, pAllocator, pSwapchain);
    }

    VKAPI_ATTR VkResult VKAPI_CALL xv_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
        UpdateFPS();
        return g_pfnNextQueuePresent(queue, pPresentInfo);
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetDeviceProcAddr(VkDevice device, const char* pName) {
        std::string n = pName;
        if (n == "vkEndCommandBuffer") return (PFN_vkVoidFunction)xv_vkEndCommandBuffer;
        if (n == "vkCreateRenderPass") return (PFN_vkVoidFunction)xv_vkCreateRenderPass;
        if (n == "vkCreateSwapchainKHR") return (PFN_vkVoidFunction)xv_vkCreateSwapchainKHR;
        if (n == "vkQueuePresentKHR") return (PFN_vkVoidFunction)xv_vkQueuePresentKHR;
        return nullptr;
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
        std::string n = pName;
        if (n == "vkEndCommandBuffer") return (PFN_vkVoidFunction)xv_vkEndCommandBuffer;
        if (n == "vkCreateRenderPass") return (PFN_vkVoidFunction)xv_vkCreateRenderPass;
        if (n == "vkCreateSwapchainKHR") return (PFN_vkVoidFunction)xv_vkCreateSwapchainKHR;
        if (n == "vkQueuePresentKHR") return (PFN_vkVoidFunction)xv_vkQueuePresentKHR;
        if (n == "vkGetDeviceProcAddr") return (PFN_vkVoidFunction)xv_vkGetDeviceProcAddr;
        return nullptr;
    }

    VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct) {
        pVersionStruct->pfnGetInstanceProcAddr = xv_vkGetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = xv_vkGetDeviceProcAddr;
        return VK_SUCCESS;
    }
}
