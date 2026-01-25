#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <string>
#include <chrono>
#include <vector>
#include <android/log.h>

#define SCALE_FACTOR 0.70f 
#define LOD_BIAS 2.5f
#define LOG_TAG "XVD_GOD"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

struct OverlayContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    uint32_t width = 0, height = 0;
    float fps = 0.0f;
    int frameCount = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
    bool isInitialized = false;
};

OverlayContext g_Overlay;
bool g_FrameReady = false;

PFN_vkGetInstanceProcAddr g_pfnGetInstanceProcAddr = nullptr;
PFN_vkGetDeviceProcAddr g_pfnGetDeviceProcAddr = nullptr;
PFN_vkCreateSwapchainKHR g_pfnCreateSwapchain = nullptr;
PFN_vkCmdSetViewport g_pfnCmdSetViewport = nullptr;

extern "C" {

    // 1. DOWNSCALING & UNLOCK FPS
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
        VkSwapchainCreateInfoKHR modInfo = *pCreateInfo;
        modInfo.imageExtent.width = (uint32_t)(pCreateInfo->imageExtent.width * SCALE_FACTOR);
        modInfo.imageExtent.height = (uint32_t)(pCreateInfo->imageExtent.height * SCALE_FACTOR);
        modInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; 
        g_Overlay.width = modInfo.imageExtent.width;
        g_Overlay.height = modInfo.imageExtent.height;
        return g_pfnCreateSwapchain(device, &modInfo, pAllocator, pSwapchain);
    }

    // 2. SUPERAÇÃO: Injeção de Extensões
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice phys, const char* pLayerName, uint32_t* pPropCount, VkExtensionProperties* pProps) {
        PFN_vkEnumerateDeviceExtensionProperties pfn = (PFN_vkEnumerateDeviceExtensionProperties)g_pfnGetInstanceProcAddr(g_Overlay.instance, "vkEnumerateDeviceExtensionProperties");
        VkResult res = pfn(phys, pLayerName, pPropCount, pProps);

        if (res == VK_SUCCESS && pProps && pPropCount && *pPropCount > 2) {
            strncpy(pProps[0].extensionName, "VK_EXT_texture_compression_astc_hdr", VK_MAX_EXTENSION_NAME_SIZE);
            strncpy(pProps[1].extensionName, "VK_EXT_custom_border_color", VK_MAX_EXTENSION_NAME_SIZE);
        }
        return res;
    }

    // 3. PERFORMANCE: Sampler Xtreme & AF Bypass
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateSampler(VkDevice dev, const VkSamplerCreateInfo* pInfo, const VkAllocationCallbacks* pAlloc, VkSampler* pSamp) {
        VkSamplerCreateInfo mod = *pInfo;
        mod.mipLodBias += LOD_BIAS; 
        mod.anisotropyEnable = VK_FALSE; 
        PFN_vkCreateSampler pfn = (PFN_vkCreateSampler)g_pfnGetDeviceProcAddr(dev, "vkCreateSampler");
        return pfn(dev, &mod, pAlloc, pSamp);
    }

    // 4. CORREÇÃO DE VIEWPORT
    VKAPI_ATTR void VKAPI_CALL xv_vkCmdSetViewport(VkCommandBuffer cmd, uint32_t first, uint32_t count, const VkViewport* pVp) {
        VkViewport hackedVp = *pVp;
        hackedVp.width = (float)g_Overlay.width;
        hackedVp.height = (float)g_Overlay.height;
        if (!g_pfnCmdSetViewport) g_pfnCmdSetViewport = (PFN_vkCmdSetViewport)g_pfnGetDeviceProcAddr(g_Overlay.device, "vkCmdSetViewport");
        g_pfnCmdSetViewport(cmd, first, count, &hackedVp);
    }

    // 5. OVERLAY GOD MODE
    VKAPI_ATTR void VKAPI_CALL xv_vkCmdEndRenderPass(VkCommandBuffer cmd) {
        if (g_Overlay.isInitialized) {
            ImGui_ImplVulkan_NewFrame(); ImGui::NewFrame();
            ImGui::SetNextWindowPos(ImVec2(40, 40));
            ImGui::Begin("GOD_MODE", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "XVD RIVER - GOD MODE");
            ImGui::Text("Status: Dominating Hardware");
            ImGui::Text("FPS: %.1f | Res: %.0f%%", g_Overlay.fps, SCALE_FACTOR * 100);
            ImGui::End(); ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
        }
        PFN_vkCmdEndRenderPass pfn = (PFN_vkCmdEndRenderPass)g_pfnGetDeviceProcAddr(g_Overlay.device, "vkCmdEndRenderPass");
        pfn(cmd);
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetDeviceProcAddr(VkDevice dev, const char* pName) {
        g_Overlay.device = dev;
        std::string n = pName;
        if (n == "vkCmdEndRenderPass") return (PFN_vkVoidFunction)xv_vkCmdEndRenderPass;
        if (n == "vkCmdSetViewport") return (PFN_vkVoidFunction)xv_vkCmdSetViewport;
        if (n == "vkCreateSampler") return (PFN_vkVoidFunction)xv_vkCreateSampler;
        return g_pfnGetDeviceProcAddr(dev, pName);
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetInstanceProcAddr(VkInstance inst, const char* pName) {
        g_Overlay.instance = inst;
        std::string n = pName;
        if (n == "vkEnumerateDeviceExtensionProperties") return (PFN_vkVoidFunction)xv_vkEnumerateDeviceExtensionProperties;
        if (n == "vkCreateSwapchainKHR") {
            g_pfnCreateSwapchain = (PFN_vkCreateSwapchainKHR)g_pfnGetInstanceProcAddr(inst, "vkCreateSwapchainKHR");
            return (PFN_vkVoidFunction)xv_vkCreateSwapchainKHR;
        }
        return g_pfnGetInstanceProcAddr(inst, pName);
    }

    VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVer) {
        pVer->pfnGetInstanceProcAddr = xv_vkGetInstanceProcAddr;
        pVer->pfnGetDeviceProcAddr = xv_vkGetDeviceProcAddr;
        return VK_SUCCESS;
    }
}