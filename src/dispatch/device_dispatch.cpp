#include "device_dispatch.h"
#include <unordered_map>

static std::unordered_map<VkDevice, DeviceDispatchTable> g_device_tables;
static std::unordered_map<VkQueue, VkDevice> g_queue_to_device;

void xv_init_device_dispatch(VkDevice device, PFN_vkGetDeviceProcAddr gdpa) {
    DeviceDispatchTable table = {};
    table.GetDeviceProcAddr = gdpa;
    #define LOAD_FN(name) table.name = (PFN_vk##name)gdpa(device, "vk" #name)
    LOAD_FN(CreateSwapchainKHR);
    LOAD_FN(GetSwapchainImagesKHR);
    LOAD_FN(CreateBuffer);
    LOAD_FN(GetBufferMemoryRequirements);
    LOAD_FN(AllocateMemory);
    LOAD_FN(BindBufferMemory);
    LOAD_FN(MapMemory);
    LOAD_FN(CreateCommandPool);
    LOAD_FN(AllocateCommandBuffers);
    LOAD_FN(BeginCommandBuffer);
    LOAD_FN(EndCommandBuffer);
    LOAD_FN(CmdPipelineBarrier);
    LOAD_FN(CmdCopyBufferToImage);
    LOAD_FN(QueueSubmit);
    LOAD_FN(QueuePresentKHR);
    LOAD_FN(ResetCommandPool);
    LOAD_FN(QueueWaitIdle);
    g_device_tables[device] = table;
}

DeviceDispatchTable* xv_get_device_dispatch(VkDevice device) {
    return &g_device_tables[device];
}

void xv_set_queue_mapping(VkQueue queue, VkDevice device) {
    g_queue_to_device[queue] = device;
}

DeviceDispatchTable* xv_get_queue_dispatch(VkQueue queue) {
    return &g_device_tables[g_queue_to_device[queue]];
}
