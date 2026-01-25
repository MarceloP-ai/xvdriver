#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <string>
#include <chrono>
#include <vector>
#include <android/log.h> // Para debug via Logcat

#define LOG_TAG "XVDriver"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

struct OverlayContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physDevice = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    char gpuName[256] = "GPU";
    uint32_t width = 0, height = 0;
    bool isInitialized = false;
    float fps = 0.0f;
    int frameCount = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
};

OverlayContext g_Overlay;
bool g_FrameReady = false;

PFN_vkGetDeviceProcAddr g_pfnGetDeviceProcAddr = nullptr;
PFN_vkGetInstanceProcAddr g_pfnGetInstanceProcAddr = nullptr;
PFN_vkCreateDevice g_pfnCreateDevice = nullptr;
PFN_vkCmdEndRenderPass g_pfnCmdEndRenderPass = nullptr;
PFN_vkQueuePresentKHR g_pfnQueuePresent = nullptr;
PFN_vkCreateRenderPass g_pfnCreateRenderPass = nullptr;
PFN_vkCreateSwapchainKHR g_pfnCreateSwapchain = nullptr;

void SetupImGui() {
    if (g_Overlay.isInitialized || !g_Overlay.renderPass || !g_Overlay.device) return;

    VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};
    VkDescriptorPoolCreateInfo pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(g_Overlay.device, &pool_info, nullptr, &g_Overlay.descriptorPool);

    ImGui::CreateContext();
    ImGui::GetIO().IniFilename = nullptr;
    ImGui::GetIO().FontGlobalScale = 2.5f;

    ImGui_ImplVulkan_InitInfo ii = {};
    ii.Instance = g_Overlay.instance;
    ii.PhysicalDevice = g_Overlay.physDevice;
    ii.Device = g_Overlay.device;
    ii.Queue = g_Overlay.graphicsQueue;
    ii.DescriptorPool = g_Overlay.descriptorPool;
    ii.MinImageCount = 2;
    ii.ImageCount = 3;

    struct MemLayout { void* d[7]; VkSampleCountFlagBits msaa; VkRenderPass rp; };
    MemLayout* l = (MemLayout*)&ii;
    l->msaa = VK_SAMPLE_COUNT_1_BIT;
    l->rp = g_Overlay.renderPass;

    if (ImGui_ImplVulkan_Init(&ii)) {
        g_Overlay.isInitialized = true;
        g_Overlay.lastTime = std::chrono::high_resolution_clock::now();
        LOGI("ImGui Initialized Successfully");
    }
}

extern "C" {
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
        g_Overlay.width = pCreateInfo->imageExtent.width;
        g_Overlay.height = pCreateInfo->imageExtent.height;
        // Removido o bloqueio de MAILBOX forçado para evitar crash
        return g_pfnCreateSwapchain(device, pCreateInfo, pAllocator, pSwapchain);
    }

    VKAPI_ATTR void VKAPI_CALL xv_vkCmdEndRenderPass(VkCommandBuffer commandBuffer) {
        if (g_Overlay.isInitialized && g_FrameReady) {
            ImGui_ImplVulkan_NewFrame();
            ImGui::NewFrame();
            ImGui::Begin("XVD", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
            ImGui::TextColored(ImVec4(0,1,0,1), "FPS: %.1f | %dx%d", g_Overlay.fps, g_Overlay.width, g_Overlay.height);
            ImGui::End();
            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
            g_FrameReady = false;
        }
        g_pfnCmdEndRenderPass(commandBuffer);
    }

    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
        VkResult res = g_pfnCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
        if (res == VK_SUCCESS) {
            g_Overlay.renderPass = *pRenderPass;
            SetupImGui();
        }
        return res;
    }

    VKAPI_ATTR VkResult VKAPI_CALL xv_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
        g_Overlay.frameCount++;
        auto now = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = now - g_Overlay.lastTime;
        if (elapsed.count() >= 1.0f) {
            g_Overlay.fps = g_Overlay.frameCount / elapsed.count();
            g_Overlay.frameCount = 0;
            g_Overlay.lastTime = now;
        }
        g_Overlay.graphicsQueue = queue;
        g_FrameReady = true;
        return g_pfnQueuePresent(queue, pPresentInfo);
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetDeviceProcAddr(VkDevice device, const char* pName) {
        g_Overlay.device = device;
        std::string n = pName;
        if (n == "vkCmdEndRenderPass") return (PFN_vkVoidFunction)xv_vkCmdEndRenderPass;
        if (n == "vkCreateRenderPass") return (PFN_vkVoidFunction)xv_vkCreateRenderPass;
        if (n == "vkQueuePresentKHR") return (PFN_vkVoidFunction)xv_vkQueuePresentKHR;
        if (n == "vkCreateSwapchainKHR") return (PFN_vkVoidFunction)xv_vkCreateSwapchainKHR;
        return g_pfnGetDeviceProcAddr(device, pName);
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
        g_Overlay.instance = instance;
        std::string n = pName;
        if (n == "vkGetDeviceProcAddr") return (PFN_vkVoidFunction)xv_vkGetDeviceProcAddr;
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