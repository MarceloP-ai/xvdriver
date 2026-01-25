#include <vulkan/vulkan.h>
#include <android/log.h>
#include <string.h>

#define LOG_TAG "XVD_DEBUG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" {
    // Essa função é o que o Android busca primeiro ao carregar um driver Vulkan
    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName) {
        LOGI("XVD_DEBUG: ICD Hook success for %s", pName);
        return nullptr;
    }

    // Função de fallback para o carregador
    VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
        LOGI("XVD_DEBUG: ICD version negotiated");
        *pSupportedVersion = 2;
        return VK_SUCCESS;
    }
}