#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <string>
#include <chrono>
#include <vector>
#include <android/log.h>

#define SCALE_FACTOR 0.70f 
#define LOD_BIAS 2.5f // Mais agressivo que a v6
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
PFN_vkGetInstanceProcAddr g_pfnGetInstanceProcAddr = nullptr;
PFN_vkGetDeviceProcAddr g_pfnGetDeviceProcAddr = nullptr;
PFN_vkCreateSwapchainKHR g_pfnCreateSwapchain = nullptr;

extern "C" {
    // 1. SUPERAÇÃO: Injeção de Extensões (Engana o motor do jogo sobre capacidades da GPU)
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice phys, const char* pLayerName, uint32_t* pPropCount, VkExtensionProperties* pProps) {
        PFN_vkEnumerateDeviceExtensionProperties pfn = (PFN_vkEnumerateDeviceExtensionProperties)g_pfnGetInstanceProcAddr(g_Overlay.instance, "vkEnumerateDeviceExtensionProperties");
        VkResult res = pfn(phys, pLayerName, pPropCount, pProps);

        if (pProps && pPropCount) {
            // Adicionamos extensões que a Xclipse/Mali as vezes esconde mas suporta via wrapper
            strncpy(pProps[0].extensionName, "VK_EXT_texture_compression_astc_hdr", VK_MAX_EXTENSION_NAME_SIZE);
            strncpy(pProps[1].extensionName, "VK_EXT_custom_border_color", VK_MAX_EXTENSION_NAME_SIZE);
            LOGI("XVD God: Extensions Injected");
        }
        return res;
    }

    // 2. PERFORMANCE: Sampler Xtreme
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateSampler(VkDevice dev, const VkSamplerCreateInfo* pInfo, const VkAllocationCallbacks* pAlloc, VkSampler* pSamp) {
        VkSamplerCreateInfo mod = *pInfo;
        mod.mipLodBias += LOD_BIAS; 
        mod.anisotropyEnable = VK_FALSE; // Desativar AF para ganhar 5-10% de FPS
        PFN_vkCreateSampler pfn = (PFN_vkCreateSampler)g_pfnGetDeviceProcAddr(dev, "vkCreateSampler");
        return pfn(dev, &mod, pAlloc, pSamp);
    }

    // 3. OVERLAY: Minimalista e intimidador
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
        if (n == "vkCreateSampler") return (PFN_vkVoidFunction)xv_vkCreateSampler;
        return g_pfnGetDeviceProcAddr(dev, pName);
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetInstanceProcAddr(VkInstance inst, const char* pName) {
        g_Overlay.instance = inst;
        std::string n = pName;
        if (n == "vkEnumerateDeviceExtensionProperties") return (PFN_vkVoidFunction)xv_vkEnumerateDeviceExtensionProperties;
        if (n == "vkCreateSwapchainKHR") {
            g_pfnCreateSwapchain = (PFN_vkCreateSwapchainKHR)g_pfnGetInstanceProcAddr(inst, "vkCreateSwapchainKHR");
            return (PFN_vkVoidFunction)xv_vkCreateSwapchainKHR; // Usa lógica anterior de escala
        }
        return g_pfnGetInstanceProcAddr(inst, pName);
    }

    VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVer) {
        pVer->pfnGetInstanceProcAddr = xv_vkGetInstanceProcAddr;
        pVer->pfnGetDeviceProcAddr = xv_vkGetDeviceProcAddr;
        return VK_SUCCESS;
    }
}