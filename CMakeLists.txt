cmake_minimum_required(VERSION 3.10)
project(XVDriver_Eden)
set(CMAKE_CXX_STANDARD 17)

# Definir explicitamente a pasta de inclusão como absoluta e de sistema
include_directories(SYSTEM "C:/Projeto/xvdriver/src")
include_directories("C:/Projeto/xvdriver/src")

add_library(VkLayer_xvd SHARED 
    src/vk_wrapper.cpp 
    src/imgui.cpp 
    src/imgui_draw.cpp 
    src/imgui_tables.cpp 
    src/imgui_widgets.cpp 
    src/imgui_impl_vulkan.cpp
    src/logger.cpp
    src/xv_loader.cpp
)

target_include_directories(VkLayer_xvd PUBLIC "C:/Projeto/xvdriver/src")

target_link_libraries(VkLayer_xvd android log vulkan)
