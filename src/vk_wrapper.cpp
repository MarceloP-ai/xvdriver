#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <chrono>
#include <fstream>
#include <string>
#include <map>

// Armazena o ponteiro original da fun??o de apresenta??o
PFN_vkQueuePresentKHR g_pfnNextQueuePresent = nullptr;

extern "C" {
    void WriteLog(const std::string& msg) {
        std::ofstream log("C:\\Projeto\\xvdriver\\log.txt", std::ios::app);
        if(log.is_open()) { log << msg << std::endl; }
    }

    VKAPI_ATTR VkResult VKAPI_CALL xv_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) {
        static auto lastTime = std::chrono::high_resolution_clock::now();
        static uint32_t frameCount = 0;
        frameCount++;
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> elapsed = currentTime - lastTime;
        
        if (elapsed.count() >= 1.0) {
            WriteLog("FPS: " + std::to_string(frameCount / elapsed.count()));
            frameCount = 0;
            lastTime = currentTime;
        }

        // CHAMA A FUN??O REAL (Isso evita que o jogo trave ou a layer seja ignorada)
        if (g_pfnNextQueuePresent) return g_pfnNextQueuePresent(queue, pPresentInfo);
        return VK_SUCCESS; 
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetDeviceProcAddr(VkDevice device, const char* pName) {
        if (std::string(pName) == "vkQueuePresentKHR") return (PFN_vkVoidFunction)xv_vkQueuePresentKHR;
        return nullptr;
    }

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
        if (std::string(pName) == "vkGetInstanceProcAddr") return (PFN_vkVoidFunction)xv_vkGetInstanceProcAddr;
        if (std::string(pName) == "vkGetDeviceProcAddr") return (PFN_vkVoidFunction)xv_vkGetDeviceProcAddr;
        if (std::string(pName) == "vkQueuePresentKHR") return (PFN_vkVoidFunction)xv_vkQueuePresentKHR;
        return nullptr;
    }

    VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface* pVersionStruct) {
        uint32_t* pVersion = (uint32_t*)pVersionStruct;
        if (*pVersion >= 2) {
            *pVersion = 2;
            pVersionStruct->pfnGetInstanceProcAddr = xv_vkGetInstanceProcAddr;
            pVersionStruct->pfnGetDeviceProcAddr = xv_vkGetDeviceProcAddr;
            pVersionStruct->pfnGetPhysicalDeviceProcAddr = nullptr;

            // Busca o ponteiro real do driver para o Present
            if (pVersionStruct->pfnGetInstanceProcAddr) {
                g_pfnNextQueuePresent = (PFN_vkQueuePresentKHR)pVersionStruct->pfnGetDeviceProcAddr(NULL, "vkQueuePresentKHR");
            }
            
            WriteLog("Layer engatada na corrente Vulkan!");
        }
        return VK_SUCCESS;
    }
}