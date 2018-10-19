#ifndef UTILS
#define UTILS

#include <vector>
#include <optional>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

VkResult createDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pCallback);
void destroyDebugUtilsMessengerEXT(
    VkInstance instance,
    VkDebugUtilsMessengerEXT pCallback,
    const VkAllocationCallbacks* pAllocator);

namespace glm {
    template<typename T>
    mat4 vulkanPerspective(T fovy, T aspect, T zNear, T zFar) {
        mat4 proj = perspective(fovy, aspect, zNear, zFar);
        proj[1][1] *= -1;
        return proj;
    }
}

constexpr uint32_t kilo = 1024;
constexpr uint32_t mega = 1024 * kilo;
constexpr uint32_t giga = 1024 * mega;

#endif