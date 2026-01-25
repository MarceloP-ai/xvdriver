cmake_minimum_required(VERSION 3.10)
project(xvdriver)

set(CMAKE_CXX_STANDARD 17)

include_directories(include include/imgui)

# Lista de arquivos fonte
set(SOURCES 
    src/vk_wrapper.cpp
    src/imgui/imgui.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_impl_vulkan.cpp
)

add_library(xvdriver SHARED ${SOURCES})

# Configuração específica por Plataforma
if(ANDROID)
    # Remove definições de Windows e adiciona as de Android
    target_compile_definitions(xvdriver PRIVATE VK_USE_PLATFORM_ANDROID_KHR)
    target_link_libraries(xvdriver vulkan android log)
else()
    # Configuração para Windows
    target_compile_definitions(xvdriver PRIVATE VK_USE_PLATFORM_WIN32_KHR)
    target_link_libraries(xvdriver vulkan)
endif()