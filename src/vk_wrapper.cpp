#include <vulkan/vulkan.h>
#include <stdio.h>

extern "C" {
    // Construtor: Executa no momento exato em que a biblioteca é carregada
    void __attribute__((constructor)) init() {
        // Tenta gravar na pasta Download (acesso comum no Android)
        FILE* f = fopen("/sdcard/Download/xvd_test.txt", "w");
        if (f) {
            fprintf(f, "XVDriver v13: Injected successfully at system level.");
            fclose(f);
        }
    }

    // Mantendo as assinaturas básicas para o Eden não rejeitar
    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName) {
        return nullptr;
    }

    VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
        *pSupportedVersion = 2;
        return VK_SUCCESS;
    }
}