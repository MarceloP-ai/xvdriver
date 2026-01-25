#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_win32.h>
#include <string>
#include <chrono>

struct OverlayContext {
    VkInstance instance = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    VkPhysicalDevice physDevice = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
    bool isInitialized = false;
    float fps = 0.0f;
};

OverlayContext g_Overlay;
PFN_vkGetDeviceProcAddr g_pfnGetDeviceProcAddr = nullptr;
PFN_vkGetInstanceProcAddr g_pfnGetInstanceProcAddr = nullptr;
PFN_vkCreateDevice g_pfnCreateDevice = nullptr;
PFN_vkCmdEndRenderPass g_pfnCmdEndRenderPass = nullptr;
PFN_vkQueuePresentKHR g_pfnQueuePresent = nullptr;
PFN_vkCreateRenderPass g_pfnCreateRenderPass = nullptr;

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

void SetupImGui() {
    if (g_Overlay.isInitialized || !g_Overlay.renderPass || !g_Overlay.device || !g_Overlay.physDevice) return;

    VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};
    VkDescriptorPoolCreateInfo pool_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = pool_sizes;
    vkCreateDescriptorPool(g_Overlay.device, &pool_info, nullptr, &g_Overlay.descriptorPool);

    ImGui::CreateContext();
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

    ImGui_ImplVulkan_Init(&ii);
    g_Overlay.isInitialized = true;
}

extern "C" {
    VKAPI_ATTR void VKAPI_CALL xv_vkCmdEndRenderPass(VkCommandBuffer commandBuffer) {
        if (g_Overlay.isInitialized) {
            ImGui_ImplVulkan_NewFrame();
            ImGui::NewFrame();
            ImGui::SetNextWindowPos(ImVec2(20, 20));
            ImGui::Begin("XVDriver", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "XVDriver TEST VERSION");
            ImGui::Text("FPS: %.1f", g_Overlay.fps);
            ImGui::End();
            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
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

    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
        g_Overlay.physDevice = physicalDevice;
        return g_pfnCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    }

    VKAPI_ATTR VkResult VKAPI_CALL xv_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
        UpdateFPS();
        g_Overlay.graphicsQueue = queue;
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
        return nullptr;
    }

    VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct) {
        pVersionStruct->pfnGetInstanceProcAddr = xv_vkGetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = xv_vkGetDeviceProcAddr;
        return VK_SUCCESS;
    }
}