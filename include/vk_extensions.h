#pragma once

#include <vulkan/vulkan.h>

// Namespace para organizar funções de extensão
namespace vk_ext {

    // Verifica se uma extensão está presente na lista de dispositivos
    bool isExtensionSupported(
        const VkPhysicalDevice& device,
        const char* extensionName
    );

    // Habilita e retorna/gera a lista de extensões que você quer forçar
    VkResult getEnabledExtensions(
        const VkPhysicalDevice& device,
        uint32_t* pCount,
        const char** ppExtensions
    );

} // namespace vk_ext