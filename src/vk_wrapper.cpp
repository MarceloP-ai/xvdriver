// ... [Mantenha os includes e a struct OverlayContext iguais ao anterior]

// Função auxiliar para escolher o melhor modo de performance disponível
VkPresentModeKHR ChooseBestPresentMode(VkPhysicalDevice physDev, VkSurfaceKHR surface) {
    uint32_t count = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physDev, surface, &count, nullptr);
    std::vector<VkPresentModeKHR> modes(count);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physDev, surface, &count, modes.data());

    // 1. Tenta MAILBOX (Melhor para latência e sem tearing)
    for (const auto& m : modes) if (m == VK_PRESENT_MODE_MAILBOX_KHR) return m;
    
    // 2. Tenta IMMEDIATE (Desbloqueio total, pode ter tearing)
    for (const auto& m : modes) if (m == VK_PRESENT_MODE_IMMEDIATE_KHR) return m;

    // 3. FIFO é o padrão obrigatório (VSync ON)
    return VK_PRESENT_MODE_FIFO_KHR;
}

extern "C" {
    // INTERCEPÇÃO DE SWAPCHAIN COM SELEÇÃO INTELIGENTE
    VKAPI_ATTR VkResult VKAPI_CALL xv_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) {
        VkSwapchainCreateInfoKHR modifiedInfo = *pCreateInfo;
        
        // Detecta o melhor modo para performance baseado no hardware real
        VkPresentModeKHR bestMode = ChooseBestPresentMode(g_Overlay.physDevice, pCreateInfo->surface);
        modifiedInfo.presentMode = bestMode;

        // Ajuste de Buffers: Se for Mailbox, geralmente precisamos de pelo menos 3 imagens
        if (bestMode == VK_PRESENT_MODE_MAILBOX_KHR && modifiedInfo.minImageCount < 3) {
            modifiedInfo.minImageCount = 3; 
        }

        return g_pfnCreateSwapchain(device, &modifiedInfo, pAllocator, pSwapchain);
    }

    // ... [Mantenha as outras funções xv_vk... iguais]

    VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL xv_vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
        g_Overlay.instance = instance;
        std::string n = pName;
        
        // Precisamos capturar o vkGetPhysicalDeviceSurfacePresentModesKHR para a nossa verificação
        if (n == "vkCreateSwapchainKHR") {
            g_pfnCreateSwapchain = (PFN_vkCreateSwapchainKHR)g_pfnGetInstanceProcAddr(instance, "vkCreateSwapchainKHR");
            return (PFN_vkVoidFunction)xv_vkCreateSwapchainKHR;
        }

        return g_pfnGetInstanceProcAddr(instance, pName);
    }
}