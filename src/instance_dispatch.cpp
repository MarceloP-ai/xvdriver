#include "instance_dispatch.h"
#include <unordered_map>

static std::unordered_map<VkInstance, InstanceDispatchTable> g_instance_tables;

void xv_init_instance_dispatch(VkInstance instance, PFN_vkGetInstanceProcAddr gipa) {
    InstanceDispatchTable table = {};
    table.GetInstanceProcAddr = gipa;
    table.GetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)gipa(instance, "vkGetPhysicalDeviceMemoryProperties");
    g_instance_tables[instance] = table;
}

InstanceDispatchTable* xv_get_instance_dispatch(VkInstance instance) {
    auto it = g_instance_tables.find(instance);
    return (it != g_instance_tables.end()) ? &it->second : nullptr;
}
