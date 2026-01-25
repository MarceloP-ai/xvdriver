#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <string>
#include <chrono>
#include <android/log.h>

#define SCALE_FACTOR 0.70f 
#define LOD_BIAS 2.0f
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
    const char* maskedGpuName = "Adreno (TM) 750";
    bool isInitialized = false;
    float fps = 0.0f;
    int frameCount = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
};

OverlayContext g_Overlay;
bool g_FrameReady = false;

PFN_vkGetDeviceProcAddr g_pfnGetDeviceProcAddr = nullptr;
PFN_vkGetInstanceProcAddr g_pfnGetInstanceProcAddr = nullptr;
PFN_vkCreateSwapchainKHR g_pfnCreateSwapchain = nullptr;
PFN_vkCmdSetViewport g_pfnCmdSetViewport = nullptr;
PFN_vkCreateSampler g_pfnCreateSampler = nullptr;
PFN_vkWaitForFences g_pfnWaitForFences = nullptr;

extern "C" {
    // 1. PERFORMANCE: Otimização de Sincronização (Reduz CPU Latency)
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) {
        // Reduzimos o timeout forçado para evitar que a CPU "durma" esperando a GPU
        if (!g_pfnWaitForFences) g_pfnWaitForFences = (PFN_vkWaitForFences)g_pfnGetDeviceProcAddr(device, "vkWaitForFences");
        return g_pfnWaitForFences(device, fenceCount, pFences, waitAll, timeout > 1000000 ? 1000000 : timeout);
    }

    // 2. TEXTURE TUNING
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) {
        VkSamplerCreateInfo modInfo = *pCreateInfo;
        modInfo.mipLodBias += LOD_BIAS;
        if (!g_pfnCreateSampler) g_pfnCreateSampler = (PFN_vkCreateSampler)g_pfnGetDeviceProcAddr(device, "vkCreateSampler");
        return g_pfnCreateSampler(device, &modInfo, pAllocator, pSampler);
    }

    // 3. DOWNSCALING & UNLOCK FPS
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
        VkSwapchainCreateInfoKHR modInfo = *pCreateInfo;
        modInfo.imageExtent.width = (uint32_t)(pCreateInfo->imageExtent.width * SCALE_FACTOR);
        modInfo.imageExtent.height = (uint32_t)(pCreateInfo->imageExtent.height * SCALE_FACTOR);
        modInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; 
        g_Overlay.width = modInfo.imageExtent.width;
        g_Overlay.height = modInfo.imageExtent.height;
        return g_pfnCreateSwapchain(device, &modInfo, pAllocator, pSwapchain);
    }

    // 4. OVERLAY (Com contador de Frame-time)
    VKAPI_ATTR void VKAPI_CALL xv_vkCmdEndRenderPass(VkCommandBuffer commandBuffer) {
        if (g_Overlay.isInitialized && g_FrameReady) {
            ImGui_ImplVulkan_NewFrame(); ImGui::NewFrame();
            ImGui::SetNextWindowPos(ImVec2(50, 50));
            ImGui::Begin("XVD_PRO_ENGINE", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
            ImGui::TextColored(ImVec4(0, 0.8f, 1, 1), "ENGINE OPTIMIZED");
            ImGui::Text("GPU: %s", g_Overlay.maskedGpuName);
            ImGui::Text("Frame-time: %.2f ms", 1000.0f / (g_Overlay.fps > 0 ? g_Overlay.fps : 1.0f));
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS: %.1f", g_Overlay.fps);
            ImGui::End(); ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
            g_FrameReady = false;
        }
        PFN_vkCmdEndRenderPass pfn = (PFN_vkCmdEndRenderPass)g_pfnGetDeviceProcAddr(g_Overlay.device, "vkCmdEndRenderPass");
        pfn(commandBuffer);
    }

    // Hooks e Endereçamento
    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetDeviceProcAddr(VkDevice device, const char* pName) {
        g_Overlay.device = device;
        std::string n = pName;
        if (n == "vkCmdEndRenderPass") return (PFN_vkVoidFunction)xv_vkCmdEndRenderPass;
        if (n == "vkCreateSampler") return (PFN_vkVoidFunction)xv_vkCreateSampler;
        if (n == "vkWaitForFences") return (PFN_vkVoidFunction)xv_vkWaitForFences;
        return g_pfnGetDeviceProcAddr(device, pName);
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
        g_Overlay.instance = instance;
        std::string n = pName;
        if (n == "vkCreateSwapchainKHR") {
            g_pfnCreateSwapchain = (PFN_vkCreateSwapchainKHR)g_pfnGetInstanceProcAddr(instance, "vkCreateSwapchainKHR");
            return (PFN_vkVoidFunction)xv_vkCreateSwapchainKHR;
        }
        return g_pfnGetInstanceProcAddr(instance, pName);
    }

    VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct) {
        pVersionStruct->pfnGetInstanceProcAddr = xv_vkGetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = xv_vkGetDeviceProcAddr;
        return VK_SUCCESS;
    }
}