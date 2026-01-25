cmake_minimum_required(VERSION 3.22.1)

project(xvdriver LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Caminhos de Inclusão
include_directories(
    include
    include/imgui
)

# Definições para Android e Vulkan
add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)

# Localizar todos os arquivos fonte (Driver + ImGui)
file(GLOB IMGUI_SOURCES 
    "src/imgui/imgui.cpp"
    "src/imgui/imgui_draw.cpp"
    "src/imgui/imgui_widgets.cpp"
    "src/imgui/imgui_tables.cpp"
    "src/imgui/imgui_impl_vulkan.cpp"
)

set(SOURCES 
    src/vk_wrapper.cpp
    ${IMGUI_SOURCES}
)

# Criar a biblioteca compartilhada (.so)
add_library(xvdriver SHARED ${SOURCES})

# Linkar bibliotecas essenciais: 
# log = Debug no Android (Logcat)
# vulkan = API Gráfica
target_link_libraries(xvdriver 
    log 
    vulkan
)

# Configurações de saída para o NDK
set_target_properties(xvdriver PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)