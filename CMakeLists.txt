cmake_minimum_required(VERSION 3.10)
project(xvdriver)

set(CMAKE_CXX_STANDARD 17)

# Encontrar Vulkan
find_package(Vulkan REQUIRED)

# Incluir diretórios
include_directories(${Vulkan_INCLUDE_DIRS})
include_directories(include)
include_directories(include/imgui) # Novo include para ImGui

# Definir arquivos do ImGui
set(IMGUI_SOURCES
    src/imgui/imgui.cpp
    src/imgui/imgui_demo.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/imgui_impl_vulkan.cpp
    src/imgui/imgui_impl_win32.cpp
)

# Definir a DLL
add_library(xvdriver SHARED src/vk_wrapper.cpp ${IMGUI_SOURCES} xvdriver.def)

# Linkar bibliotecas
target_link_libraries(xvdriver ${Vulkan_LIBRARIES})

# Definições necessárias para o ImGui não conflitar com Windows
target_compile_definitions(xvdriver PRIVATE VK_USE_PLATFORM_WIN32_KHR)