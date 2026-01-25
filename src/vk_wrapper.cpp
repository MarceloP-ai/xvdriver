#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <string>
#include <chrono>
#include <vector>

#ifdef _WIN32
#include <imgui_impl_win32.h>
#endif

struct OverlayContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physDevice = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    
    char gpuName[256] = "Generic GPU";
    bool isInitialized = false;
    float fps = 0.0f;
    float frameTime = 0.0f;
    int frameCount = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
    std::vector<float> frameHistory;
};

OverlayContext g_Overlay;
bool g_FrameReady = false;

PFN_vkGetDeviceProcAddr g_pfnGetDeviceProcAddr = nullptr;
PFN_vkGetInstanceProcAddr g_pfnGetInstanceProcAddr = nullptr;
PFN_vkCreateDevice g_pfnCreateDevice = nullptr;
PFN_vkCmdEndRenderPass g_pfnCmdEndRenderPass = nullptr;
PFN_vkQueuePresentKHR g_pfnQueuePresent = nullptr;
PFN_vkCreateRenderPass g_pfnCreateRenderPass = nullptr;

void UpdateMetrics() {
    g_Overlay.frameCount++;
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = currentTime - g_Overlay.lastTime;

    if (elapsed.count() >= 0.5f) {
        g_Overlay.fps = g_Overlay.frameCount / elapsed.count();
        g_Overlay.frameTime = (elapsed.count() / g_Overlay.frameCount) * 1000.0f;
        
        g_Overlay.frameHistory.push_back(g_Overlay.fps);
        if (g_Overlay.frameHistory.size() > 50) g_Overlay.frameHistory.erase(g_Overlay.frameHistory.begin());
        
        g_Overlay.frameCount = 0;
        g_Overlay.lastTime = currentTime;
    }
}

void SetupImGui() {
    if (g_Overlay.isInitialized || !g_Overlay.renderPass || !g_Overlay.device) return;

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(g_Overlay.physDevice, &props);
    strncpy(g_Overlay.gpuName, props.deviceName, 256);

    VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};
    VkDescriptorPoolCreateInfo pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(g_Overlay.device, &pool_info, nullptr, &g_Overlay.descriptorPool);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.FontGlobalScale = 2.5f; // Fonte maior para Android

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
    }
}

extern "C" {
    VKAPI_ATTR void VKAPI_CALL xv_vkCmdEndRenderPass(VkCommandBuffer commandBuffer) {
        if (g_Overlay.isInitialized && g_FrameReady) {
            ImGui_ImplVulkan_NewFrame();
            ImGui::NewFrame();
            
            // Lógica de Cores Dinâmicas
            ImVec4 statusColor;
            if (g_Overlay.fps >= 55.0f) statusColor = ImVec4(0, 1, 0, 1);      // Verde
            else if (g_Overlay.fps >= 30.0f) statusColor = ImVec4(1, 1, 0, 1); // Amarelo
            else statusColor = ImVec4(1, 0, 0, 1);                           // Vermelho

            ImGui::SetNextWindowPos(ImVec2(40, 40), ImGuiCond_FirstUseEver);
            ImGui::Begin("XVDriver_Elite", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
            
            ImGui::TextColored(statusColor, "GPU: %s", g_Overlay.gpuName);
            ImGui::Separator();
            ImGui::TextColored(statusColor, "FPS: %.1f", g_Overlay.fps);
            ImGui::Text("FT: %.2f ms", g_Overlay.frameTime);
            
            if (!g_Overlay.frameHistory.empty()) {
                ImGui::PushStyleColor(ImGuiCol_PlotLines, statusColor);
                ImGui::PlotLines("##graph", g_Overlay.frameHistory.data(), g_Overlay.frameHistory.size(), 0, nullptr, 0, 120, ImVec2(250, 50));
                ImGui::PopStyleColor();
            }

            ImGui::End();
            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
            g_FrameReady = false;
        }
        g_pfnCmdEndRenderPass(commandBuffer);
    }

    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkRenderPass* pRenderPass) {
        VkResult res = g_pfnCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
        if (res == VK_SUCCESS && !g_Overlay.isInitialized) {
            g_Overlay.renderPass = *pRenderPass;
            SetupImGui();
        }
        return res;
    }

    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
        g_Overlay.physDevice = physicalDevice;
        return g_pfnCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    }

    VKAPI_ATTR VkResult VKAPI_CALL xv_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
        UpdateMetrics();
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
        return g_pfnGetDeviceProcAddr(device, pName);
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
        g_Overlay.instance = instance;
        std::string n = pName;
        if (n == "vkCreateDevice") return (PFN_vkVoidFunction)xv_vkCreateDevice;
        if (n == "vkGetDeviceProcAddr") return (PFN_vkVoidFunction)xv_vkGetDeviceProcAddr;
        return g_pfnGetInstanceProcAddr(instance, pName);
    }

    VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct) {
        pVersionStruct->pfnGetInstanceProcAddr = xv_vkGetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = xv_vkGetDeviceProcAddr;
        return VK_SUCCESS;
    }
}