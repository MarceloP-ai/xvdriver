cmake_minimum_required(VERSION 3.10)
project(XVDriver_Eden)
set(CMAKE_CXX_STANDARD 17)

add_library(VkLayer_xvd SHARED 
    src/vk_wrapper.cpp 
    src/imgui.cpp 
    src/imgui_draw.cpp 
    src/imgui_tables.cpp 
    src/imgui_widgets.cpp 
    src/imgui_impl_vulkan.cpp
)

# target_include_directories com SYSTEM força o compilador a tratar a pasta como biblioteca
target_include_directories(VkLayer_xvd SYSTEM PRIVATE src)
target_include_directories(VkLayer_xvd PRIVATE src)

set_target_properties(VkLayer_xvd PROPERTIES 
    OUTPUT_NAME "VkLayer_xvd"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build_android"
)

target_link_libraries(VkLayer_xvd android log vulkan)
