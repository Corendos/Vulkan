#ifndef COMMANDPOOL
#define COMMANDPOOL

#include <stdexcept>
#include <mutex>

#include <vulkan/vulkan.h>

#include "vulkan/QueueFamilyIndices.hpp"

class CommandPool {
    public:
        void create(VkDevice device, uint32_t queueFamilyIndex);
        void destroy(VkDevice device);

        VkCommandPool getHandler() const;

        void lock();
        void unlock();

    private:
        VkCommandPool mHandler;
        std::mutex mMutex;

        bool mCreated{false};
};

#endif