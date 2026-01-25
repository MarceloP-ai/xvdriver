#include <vulkan/vulkan.h>
#include <stdlib.h>
#include <android/log.h>

extern "C" {
    // Força alta performance e ignora gargalos de energia
    void __attribute__((constructor)) init() {
        setenv("VK_ICD_FILENAMES", "/vendor/lib64/hw/vulkan.mali.so", 1);
        setenv("MALI_PERF_MODE", "1", 1);
        setenv("AFBC_DISABLE", "0", 1); // Mantém compressão para poupar banda de memória
    }

    // O Eden busca esse símbolo específico para validar o driver Samsung
    VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
        *pSupportedVersion = 2;
        return VK_SUCCESS;
    }

    // Ponteiro principal que o sistema exige
    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName) {
        return nullptr; 
    }
}