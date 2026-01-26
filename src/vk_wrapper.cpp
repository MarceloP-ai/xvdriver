#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <android/log.h>
#include "imgui/imgui.h"

#define LOG_TAG "XVD_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" {
    // Função que será chamada pelo jogo a cada frame
    VKAPI_ATTR VkResult VKAPI_CALL vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
        // Futuro local do: ImGui_Render_Frame();
        
        // Por enquanto, apenas repassa para o driver real para não crashar
        return VK_SUCCESS; 
    }

    void __attribute__((constructor)) init() {
        setenv("MALI_PERF_MODE", "1", 1);
        setenv("VK_ICD_FILENAMES", "/vendor/lib64/hw/vulkan.mali.so", 1);
        setpriority(PRIO_PROCESS, 0, -20);
        
        LOGI("XVD Hook System: Ativado.");
    }

    VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
        *pSupportedVersion = 2;
        return VK_SUCCESS;
    }

    // Essa função é o "cardápio" que diz ao jogo quais funções nós controlamos
    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName) {
        if (strcmp(pName, "vkQueuePresentKHR") == 0) return (PFN_vkVoidFunction)vkQueuePresentKHR;
        return nullptr;
    }
}