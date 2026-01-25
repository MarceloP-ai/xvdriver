# XVDriver - Vulkan FPS Monitor Layer

Protótipo de uma Vulkan Layer desenvolvida em C++ para interceptação de comandos de renderização e monitoramento de performance.

## 🛠️ Especificações Técnicas
- **Linguagem:** C++17
- **API:** Vulkan SDK 1.3+
- **Hooking:** Interceptação da função `vkQueuePresentKHR` para cálculo de FPS.
- **Negotiation:** Implementação manual de `vkNegotiateLoaderLayerInterfaceVersion` para integração com o Vulkan Loader.

## 🚀 Desafios Resolvidos
- **Pointer Casting:** Ajuste na negociação de interface para compatibilidade com o Loader (uint32_t casting).
- **Dispatch Chain:** Lógica de salvamento de ponteiros originais (Function Hooking).
- **Registry Management:** Configuração de camadas explícitas via Registro do Windows (HKLM/HKCU).

## 📂 Estrutura
- `src/vk_wrapper.cpp`: Lógica principal da layer e hooks.
- `bin/xvdriver.json`: Manifesto de configuração para o Vulkan.
