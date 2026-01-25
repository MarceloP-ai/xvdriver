#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <string>
#include <chrono>

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
    bool isInitialized = false;
    float fps = 0.0f;
    int frameCount = 0;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime;
};

OverlayContext g_Overlay;
bool g_FrameReady = false;

// Ponteiros de função Vulkan
PFN_vkGetDeviceProcAddr g_pfnGetDeviceProcAddr = nullptr;
PFN_vkGetInstanceProcAddr g_pfnGetInstanceProcAddr = nullptr;
PFN_vkCreateDevice g_pfnCreateDevice = nullptr;
PFN_vkCmdEndRenderPass g_pfnCmdEndRenderPass = nullptr;
PFN_vkQueuePresentKHR g_pfnQueuePresent = nullptr;
PFN_vkCreateRenderPass g_pfnCreateRenderPass = nullptr;

void UpdatePerformanceMetrics() {
    g_Overlay.frameCount++;
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = currentTime - g_Overlay.lastTime;

    if (elapsed.count() >= 0.5f) { // Atualiza a cada 500ms para suavizar a leitura
        g_Overlay.fps = g_Overlay.frameCount / elapsed.count();
        g_Overlay.frameCount = 0;
        g_Overlay.lastTime = currentTime;
    }
}

void SetupImGui() {
    if (g_Overlay.isInitialized || !g_Overlay.renderPass || !g_Overlay.device || !g_Overlay.physDevice) return;

    // Pool de descritores otimizada para mobile
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }
    };
    VkDescriptorPoolCreateInfo pool_info = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = pool_sizes;

    if (vkCreateDescriptorPool(g_Overlay.device, &pool_info, nullptr, &g_Overlay.descriptorPool) != VK_SUCCESS) return;

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // Evita IO de arquivo no Android para mais fluidez

    ImGui_ImplVulkan_InitInfo ii = {};
    ii.Instance = g_Overlay.instance;
    ii.PhysicalDevice = g_Overlay.physDevice;
    ii.Device = g_Overlay.device;
    ii.Queue = g_Overlay.graphicsQueue;
    ii.DescriptorPool = g_Overlay.descriptorPool;
    ii.MinImageCount = 2;
    ii.ImageCount = 3;
    ii.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    // Layout de memória compatível para evitar crashes de compilação
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
        // Só renderiza o overlay se o frame estiver pronto e não houver conflito de shader
        if (g_Overlay.isInitialized && g_FrameReady) {
            ImGui_ImplVulkan_NewFrame();
            ImGui::NewFrame();
            
            ImGui::SetNextWindowPos(ImVec2(10, 10));
            ImGui::Begin("XVDriver_Perf", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs);
            
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "XVDriver Performance Mode");
            ImGui::Text("FPS: %.1f", g_Overlay.fps);
            
            ImGui::End();
            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
            
            g_FrameReady = false; // Consome o sinal de prontidão
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
        UpdatePerformanceMetrics();
        g_Overlay.graphicsQueue = queue;
        g_FrameReady = true; // Sinaliza que o próximo passe pode receber o overlay
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