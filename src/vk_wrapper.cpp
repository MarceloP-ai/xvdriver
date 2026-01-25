#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <string>
#include <chrono>
#include <vector>
#include <android/log.h>

#define SCALE_FACTOR 0.70f // 30% menos carga na GPU
#define LOG_TAG "XVDriver_Core"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

struct OverlayContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physDevice = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    uint32_t width = 0, height = 0;
    uint32_t realW = 0, realH = 0;
    bool isInitialized = false;
    float fps = 0.0f;
    int frameCount = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
};

OverlayContext g_Overlay;
bool g_FrameReady = false;

// Ponteiros das funções originais
PFN_vkGetDeviceProcAddr g_pfnGetDeviceProcAddr = nullptr;
PFN_vkGetInstanceProcAddr g_pfnGetInstanceProcAddr = nullptr;
PFN_vkCreateDevice g_pfnCreateDevice = nullptr;
PFN_vkCmdEndRenderPass g_pfnCmdEndRenderPass = nullptr;
PFN_vkQueuePresentKHR g_pfnQueuePresent = nullptr;
PFN_vkCreateRenderPass g_pfnCreateRenderPass = nullptr;
PFN_vkCreateSwapchainKHR g_pfnCreateSwapchain = nullptr;
PFN_vkCmdSetViewport g_pfnCmdSetViewport = nullptr;

extern "C" {
    // 1. DRIVER OVERRIDE: Manipulação da Swapchain (Downscaling + Unlock FPS)
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
        VkSwapchainCreateInfoKHR modifiedInfo = *pCreateInfo;

        g_Overlay.realW = pCreateInfo->imageExtent.width;
        g_Overlay.realH = pCreateInfo->imageExtent.height;
        
        // Aplica Downscale
        modifiedInfo.imageExtent.width = (uint32_t)(g_Overlay.realW * SCALE_FACTOR);
        modifiedInfo.imageExtent.height = (uint32_t)(g_Overlay.realH * SCALE_FACTOR);
        
        // Força modo de performance (Ignora V-Sync do jogo)
        modifiedInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; 

        g_Overlay.width = modifiedInfo.imageExtent.width;
        g_Overlay.height = modifiedInfo.imageExtent.height;

        LOGI("Driver Hack: Scaled %dx%d -> %dx%d | V-Sync OFF", g_Overlay.realW, g_Overlay.realH, g_Overlay.width, g_Overlay.height);
        return g_pfnCreateSwapchain(device, &modifiedInfo, pAllocator, pSwapchain);
    }

    // 2. DRIVER OVERRIDE: Forçar Viewport (Garante tela cheia)
    VKAPI_ATTR void VKAPI_CALL xv_vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount, const VkViewport* pViewports) {
        VkViewport hackedViewport = *pViewports;
        hackedViewport.width = (float)g_Overlay.width;
        hackedViewport.height = (float)g_Overlay.height;
        g_pfnCmdSetViewport(commandBuffer, firstViewport, viewportCount, &hackedViewport);
    }

    VKAPI_ATTR void VKAPI_CALL xv_vkCmdEndRenderPass(VkCommandBuffer commandBuffer) {
        if (g_Overlay.isInitialized && g_FrameReady) {
            ImGui_ImplVulkan_NewFrame();
            ImGui::NewFrame();
            ImGui::SetNextWindowPos(ImVec2(60, 60), ImGuiCond_FirstUseEver);
            ImGui::Begin("XV_DRIVER_CORE", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
            ImGui::TextColored(ImVec4(0,1,0,1), "XV-CORE ACTIVE");
            ImGui::Text("Render: %dx%d (%.0f%%)", g_Overlay.width, g_Overlay.height, SCALE_FACTOR * 100);
            ImGui::Text("FPS: %.1f", g_Overlay.fps);
            ImGui::End();
            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
            g_FrameReady = false;
        }
        g_pfnCmdEndRenderPass(commandBuffer);
    }

    // [Funções de Inicialização e Hook seguem o padrão anterior]
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
        VkResult res = g_pfnCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
        if (res == VK_SUCCESS) { g_Overlay.renderPass = *pRenderPass; /* SetupImGui aqui */ }
        return res;
    }

    VKAPI_ATTR VkResult VKAPI_CALL xv_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
        g_Overlay.frameCount++;
        auto now = std::chrono::high_resolution_clock::now();
        if (std::chrono::duration<float>(now - g_Overlay.lastTime).count() >= 1.0f) {
            g_Overlay.fps = g_Overlay.frameCount;
            g_Overlay.frameCount = 0;
            g_Overlay.lastTime = now;
        }
        g_FrameReady = true;
        return g_pfnQueuePresent(queue, pPresentInfo);
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetDeviceProcAddr(VkDevice device, const char* pName) {
        std::string n = pName;
        if (n == "vkCmdEndRenderPass") return (PFN_vkVoidFunction)xv_vkCmdEndRenderPass;
        if (n == "vkCmdSetViewport") return (PFN_vkVoidFunction)xv_vkCmdSetViewport;
        if (n == "vkCreateSwapchainKHR") return (PFN_vkVoidFunction)xv_vkCreateSwapchainKHR;
        if (n == "vkQueuePresentKHR") return (PFN_vkVoidFunction)xv_vkQueuePresentKHR;
        return g_pfnGetDeviceProcAddr(device, pName);
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
        std::string n = pName;
        if (n == "vkGetDeviceProcAddr") return (PFN_vkVoidFunction)xv_vkGetDeviceProcAddr;
        return g_pfnGetInstanceProcAddr(instance, pName);
    }

    VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct) {
        pVersionStruct->pfnGetInstanceProcAddr = xv_vkGetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = xv_vkGetDeviceProcAddr;
        return VK_SUCCESS;
    }
}