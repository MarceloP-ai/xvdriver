#include <vulkan/vulkan.h>
#include <android/log.h>

#define LOG_TAG "XVD_DEBUG"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" {
    // Forçamos o log assim que a biblioteca é carregada na memória do Eden
    void __attribute__((constructor)) init() {
        LOGI("XVD_DEBUG: DRIVER LOADED INTO MEMORY!");
    }

    // Interceptador de funções do sistema
    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
        LOGI("XVD_DEBUG: Eden requested function: %s", pName);
        return nullptr; // Deixa o sistema seguir após logarmos
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char* pName) {
        return nullptr;
    }
}