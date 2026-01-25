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
#define LOG_TAG "XVD_DEBUG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct OverlayContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    uint32_t width = 0, height = 0;
    float fps = 0.0f;
    bool isInitialized = false;
};

OverlayContext g_Overlay;
PFN_vkGetInstanceProcAddr g_pfnGetInstanceProcAddr = nullptr;
PFN_vkGetDeviceProcAddr g_pfnGetDeviceProcAddr = nullptr;
PFN_vkCreateSwapchainKHR g_pfnCreateSwapchain = nullptr;

extern "C" {

    // 1. DEBUG: Monitorar quais extensões o Eden está requisitando
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice phys, const char* pLayerName, uint32_t* pPropCount, VkExtensionProperties* pProps) {
        PFN_vkEnumerateDeviceExtensionProperties pfn = (PFN_vkEnumerateDeviceExtensionProperties)g_pfnGetInstanceProcAddr(g_Overlay.instance, "vkEnumerateDeviceExtensionProperties");
        VkResult res = pfn(phys, pLayerName, pPropCount, pProps);

        if (pProps && pPropCount) {
            for (uint32_t i = 0; i < *pPropCount; i++) {
                LOGI("Eden requested extension: %s", pProps[i].extensionName);
            }
            // Injeção de super-compatibilidade
            strncpy(pProps[0].extensionName, "VK_EXT_texture_compression_astc_hdr", VK_MAX_EXTENSION_NAME_SIZE);
            strncpy(pProps[1].extensionName, "VK_EXT_line_rasterization", VK_MAX_EXTENSION_NAME_SIZE);
        }
        return res;
    }

    // 2. DEBUG: Capturar falhas na criação da Swapchain
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
        LOGI("Creating Swapchain: %dx%d | MinImageCount: %d", pCreateInfo->imageExtent.width, pCreateInfo->imageExtent.height, pCreateInfo->minImageCount);
        
        VkSwapchainCreateInfoKHR modInfo = *pCreateInfo;
        modInfo.imageExtent.width = (uint32_t)(pCreateInfo->imageExtent.width * SCALE_FACTOR);
        modInfo.imageExtent.height = (uint32_t)(pCreateInfo->imageExtent.height * SCALE_FACTOR);
        modInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; 
        
        g_Overlay.width = modInfo.imageExtent.width;
        g_Overlay.height = modInfo.imageExtent.height;

        VkResult res = g_pfnCreateSwapchain(device, &modInfo, pAllocator, pSwapchain);
        if (res != VK_SUCCESS) LOGE("Swapchain Creation Failed! Result: %d", res);
        
        return res;
    }

    // Overlay com status de depuração
    VKAPI_ATTR void VKAPI_CALL xv_vkCmdEndRenderPass(VkCommandBuffer cmd) {
        if (g_Overlay.isInitialized) {
            ImGui_ImplVulkan_NewFrame(); ImGui::NewFrame();
            ImGui::SetNextWindowPos(ImVec2(40, 40));
            ImGui::Begin("XVD_DEBUG_CONSOLE", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "MODE: DEBUG & ANALYTICS");
            ImGui::Text("Render: %dx%d", g_Overlay.width, g_Overlay.height);
            ImGui::Text("GPU Intercepts: ACTIVE");
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