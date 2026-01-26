cmake_minimum_required(VERSION 3.10)
project(vulkan_samsung_hook)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden -fPIC -O3")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic -Wl,--gc-sections")

set(CMAKE_CXX_STANDARD 17)
include_directories(include include/imgui)

set(SOURCE_FILES
    src/vk_wrapper.cpp
    src/imgui/imgui.cpp
    src/imgui/imgui_draw.cpp
    src/imgui/imgui_widgets.cpp
    src/imgui/imgui_tables.cpp
    src/imgui/imgui_impl_vulkan.cpp
)

add_library(vulkan.samsung SHARED ${SOURCE_FILES})
set_target_properties(vulkan.samsung PROPERTIES PREFIX "")

# Linkagem crucial com libdl para o hook funcionar
target_link_libraries(vulkan.samsung android log dl)