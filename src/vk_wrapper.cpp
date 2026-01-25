#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <string>
#include <chrono>
#include <android/log.h>

#define SCALE_FACTOR 0.70f 
#define LOD_BIAS 2.0f  // Tuning: +2.0f deixa texturas leves (mais performance)
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

extern "C" {
    // 1. TEXTURE TUNING: Forçar texturas de alta performance
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateSampler(VkDevice device, const VkSamplerCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSampler* pSampler) {
        VkSamplerCreateInfo modInfo = *pCreateInfo;
        modInfo.mipLodBias += LOD_BIAS; // Força o uso de Mipmaps menores
        
        if (!g_pfnCreateSampler) {
            g_pfnCreateSampler = (PFN_vkCreateSampler)g_pfnGetDeviceProcAddr(device, "vkCreateSampler");
        }
        return g_pfnCreateSampler(device, &modInfo, pAllocator, pSampler);
    }

    // 2. GPU SPOOFING (Properties)
    void MaskProperties(VkPhysicalDeviceProperties* props) {
        props->vendorID = 0x5143;
        props->deviceID = 0x41445245;
        memset(props->deviceName, 0, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
        strncpy(props->deviceName, g_Overlay.maskedGpuName, VK_MAX_PHYSICAL_DEVICE_NAME_SIZE);
    }

    VKAPI_ATTR void VKAPI_CALL xv_vkGetPhysicalDeviceProperties(VkPhysicalDevice phys, VkPhysicalDeviceProperties* pProps) {
        PFN_vkGetPhysicalDeviceProperties pfn = (PFN_vkGetPhysicalDeviceProperties)g_pfnGetInstanceProcAddr(g_Overlay.instance, "vkGetPhysicalDeviceProperties");
        pfn(phys, pProps);
        MaskProperties(pProps);
    }

    // 3. DOWNSCALING & VIEWPORT (Res Fix)
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
        VkSwapchainCreateInfoKHR modInfo = *pCreateInfo;
        modInfo.imageExtent.width = (uint32_t)(pCreateInfo->imageExtent.width * SCALE_FACTOR);
        modInfo.imageExtent.height = (uint32_t)(pCreateInfo->imageExtent.height * SCALE_FACTOR);
        modInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR; 
        g_Overlay.width = modInfo.imageExtent.width;
        g_Overlay.height = modInfo.imageExtent.height;
        return g_pfnCreateSwapchain(device, &modInfo, pAllocator, pSwapchain);
    }

    VKAPI_ATTR void VKAPI_CALL xv_vkCmdSetViewport(VkCommandBuffer cmd, uint32_t first, uint32_t count, const VkViewport* pVp) {
        VkViewport hackedVp = *pVp;
        hackedVp.width = (float)g_Overlay.width;
        hackedVp.height = (float)g_Overlay.height;
        g_pfnCmdSetViewport(cmd, first, count, &hackedVp);
    }

    // 4. OVERLAY
    VKAPI_ATTR void VKAPI_CALL xv_vkCmdEndRenderPass(VkCommandBuffer commandBuffer) {
        if (g_Overlay.isInitialized && g_FrameReady) {
            ImGui_ImplVulkan_NewFrame(); ImGui::NewFrame();
            ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Always);
            ImGui::Begin("XVD_XTREME", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
            ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "XTREME TUNING ACTIVE");
            ImGui::Text("GPU: %s", g_Overlay.maskedGpuName);
            ImGui::Text("LOD Bias: +%.1f (Fast Textures)", LOD_BIAS);
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "FPS: %.1f", g_Overlay.fps);
            ImGui::End(); ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
            g_FrameReady = false;
        }
        PFN_vkCmdEndRenderPass pfn = (PFN_vkCmdEndRenderPass)g_pfnGetDeviceProcAddr(g_Overlay.device, "vkCmdEndRenderPass");
        pfn(commandBuffer);
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetDeviceProcAddr(VkDevice device, const char* pName) {
        g_Overlay.device = device;
        std::string n = pName;
        if (n == "vkCmdEndRenderPass") return (PFN_vkVoidFunction)xv_vkCmdEndRenderPass;
        if (n == "vkCmdSetViewport") return (PFN_vkVoidFunction)xv_vkCmdSetViewport;
        if (n == "vkCreateSampler") return (PFN_vkVoidFunction)xv_vkCreateSampler;
        return g_pfnGetDeviceProcAddr(device, pName);
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
        g_Overlay.instance = instance;
        std::string n = pName;
        if (n == "vkGetPhysicalDeviceProperties") return (PFN_vkVoidFunction)xv_vkGetPhysicalDeviceProperties;
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