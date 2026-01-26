#include <vulkan/vulkan.h>
#include <dlfcn.h>
#include <stdio.h>
#include <android/log.h>
#include "imgui/imgui.h"

typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_vk_icdGetInstanceProcAddr)(VkInstance instance, const char* pName);
static PFN_vk_icdGetInstanceProcAddr real_gippa = nullptr;

extern "C" {
    // Intercepta a apresentação do frame
    VKAPI_ATTR VkResult VKAPI_CALL vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
        // Aqui entrará o: if(!imgui_init) { setup_imgui(); } render_menu();
        
        // Busca a função real no driver da Samsung para não travar o jogo
        static PFN_vkQueuePresentKHR real_present = (PFN_vkQueuePresentKHR)real_gippa(nullptr, "vkQueuePresentKHR");
        return real_present(queue, pPresentInfo);
    }

    void __attribute__((constructor)) init() {
        // Carrega o driver original da Samsung/Mali em segredo
        void* handle = dlopen("/vendor/lib64/hw/vulkan.mali.so", RTLD_NOW);
        if (handle) {
            real_gippa = (PFN_vk_icdGetInstanceProcAddr)dlsym(handle, "vk_icdGetInstanceProcAddr");
        }
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_icdGetInstanceProcAddr(VkInstance instance, const char* pName) {
        if (strcmp(pName, "vkQueuePresentKHR") == 0) return (PFN_vkVoidFunction)vkQueuePresentKHR;
        
        // Se não for uma função que queremos alterar, pede para o driver real cuidar disso
        if (real_gippa) return real_gippa(instance, pName);
        return nullptr;
    }

    VKAPI_ATTR VkResult VKAPI_CALL vk_icdNegotiateLoaderICDInterfaceVersion(uint32_t* pSupportedVersion) {
        *pSupportedVersion = 2;
        return VK_SUCCESS;
    }
}