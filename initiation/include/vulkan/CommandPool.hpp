#ifndef COMMANDPOOL
#define COMMANDPOOL

#include <stdexcept>

#include <vulkan/vulkan.h>

#include "vulkan/QueueFamilyIndices.hpp"

class CommandPool {
    public:
        void create(VkDevice device, uint32_t queueFamilyIndex);
        void destroy(VkDevice device);

        VkCommandPool getHandler() const;

    private:
        VkCommandPool mHandler;

        bool mCreated{false};
};

#endif