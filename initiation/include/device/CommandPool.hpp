#ifndef COMMANDPOOL
#define COMMANDPOOL

#include <exception>

#include <vulkan/vulkan.h>

#include "device/QueueFamilyIndices.hpp"

class CommandPool {
    public:
        void create(VkDevice device, QueueFamilyIndices indices);
        void destroy(VkDevice device);

        VkCommandPool getHandler() const;

    private:
        VkCommandPool mHandler;

        bool mCreated{false};
};

#endif