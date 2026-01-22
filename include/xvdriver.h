#pragma once

// xvdriver — definitions and common includes

// Vulkan API loader intercept
#include <vulkan/vulkan.h>

// Initialize the XVDriver (to be called when loaded)
namespace xvdriver {

// Called once at load or early initialization
bool initialize();

// Called once at shutdown (if needed)
void shutdown();

} // namespace xvdriver