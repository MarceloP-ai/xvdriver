#include <vulkan/vulkan.h>
#include <android/log.h>

extern "C" {
    __attribute__((visibility("default")))
    void vk_icdGetInstanceProcAddr() {
        __android_log_write(ANDROID_LOG_ERROR, "XVD_CORE", "!!! INJETADO VIA EDEN !!!");
    }
}