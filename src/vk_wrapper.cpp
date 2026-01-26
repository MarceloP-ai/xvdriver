#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <android/log.h>

#define LOG_TAG "XVD_CORE"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" {
    void __attribute__((constructor)) init() {
        // 1. Injeção de Variáveis de Performance
        setenv("MALI_PERF_MODE", "1", 1);
        setenv("VK_ICD_FILENAMES", "/vendor/lib64/hw/vulkan.mali.so", 1);
        
        // 2. Tenta aumentar a prioridade do próprio processo (Self-Renice)
        setpriority(PRIO_PROCESS, 0, -20);
        
        LOGI("XVD Core: Otimizacoes aplicadas internamente.");
    }

    // Mantendo a compatibilidade Vulkan para o Eden aceitar o arquivo
    VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
        *pSupportedVersion = 2;
        return VK_SUCCESS;
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName) {
        return nullptr;
    }
}