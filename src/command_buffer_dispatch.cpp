#include "command_buffer_dispatch.h"
#include <unordered_map>
#include <mutex>

/* ------------------------------------------------------------------------- */
/* Dispatch table                                                             */
/* ------------------------------------------------------------------------- */

struct CommandBufferDispatch {
    PFN_vkCmdDraw        CmdDraw;
    PFN_vkCmdDrawIndexed CmdDrawIndexed;
    PFN_vkCmdDispatch    CmdDispatch;
};

static std::unordered_map<VkCommandBuffer, CommandBufferDispatch> g_dispatch;
static std::mutex g_mutex;

/* ------------------------------------------------------------------------- */
/* Init                                                                       */
/* ------------------------------------------------------------------------- */

void xv_init_command_buffer_dispatch(
    VkCommandBuffer commandBuffer,
    PFN_vkGetDeviceProcAddr gdpa)
{
    std::lock_guard<std::mutex> lock(g_mutex);

    CommandBufferDispatch table{};
    table.CmdDraw = (PFN_vkCmdDraw)
        gdpa(VK_NULL_HANDLE, "vkCmdDraw");

    table.CmdDrawIndexed = (PFN_vkCmdDrawIndexed)
        gdpa(VK_NULL_HANDLE, "vkCmdDrawIndexed");

    table.CmdDispatch = (PFN_vkCmdDispatch)
        gdpa(VK_NULL_HANDLE, "vkCmdDispatch");

    g_dispatch[commandBuffer] = table;
}

/* ------------------------------------------------------------------------- */
/* Helpers                                                                    */
/* ------------------------------------------------------------------------- */

static CommandBufferDispatch* get_dispatch(VkCommandBuffer cb)
{
    auto it = g_dispatch.find(cb);
    if (it == g_dispatch.end())
        return nullptr;
    return &it->second;
}

/* ------------------------------------------------------------------------- */
/* Interceptors                                                               */
/* ------------------------------------------------------------------------- */

VKAPI_ATTR void VKAPI_CALL xv_vkCmdDraw(
    VkCommandBuffer commandBuffer,
    uint32_t vertexCount,
    uint32_t instanceCount,
    uint32_t firstVertex,
    uint32_t firstInstance)
{
    auto* d = get_dispatch(commandBuffer);
    if (d && d->CmdDraw) {
        d->CmdDraw(
            commandBuffer,
            vertexCount,
            instanceCount,
            firstVertex,
            firstInstance);
    }
}

VKAPI_ATTR void VKAPI_CALL xv_vkCmdDrawIndexed(
    VkCommandBuffer commandBuffer,
    uint32_t indexCount,
    uint32_t instanceCount,
    uint32_t firstIndex,
    int32_t  vertexOffset,
    uint32_t firstInstance)
{
    auto* d = get_dispatch(commandBuffer);
    if (d && d->CmdDrawIndexed) {
        d->CmdDrawIndexed(
            commandBuffer,
            indexCount,
            instanceCount,
            firstIndex,
            vertexOffset,
            firstInstance);
    }
}

VKAPI_ATTR void VKAPI_CALL xv_vkCmdDispatch(
    VkCommandBuffer commandBuffer,
    uint32_t groupCountX,
    uint32_t groupCountY,
    uint32_t groupCountZ)
{
    auto* d = get_dispatch(commandBuffer);
    if (d && d->CmdDispatch) {
        d->CmdDispatch(
            commandBuffer,
            groupCountX,
            groupCountY,
            groupCountZ);
    }
}
