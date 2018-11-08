#ifndef DESCRIPTORPOOL
#define DESCRIPTORPOOL

#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.h>

class DescriptorPool {
    public:
        void create(VkDevice device);
        void destroy(VkDevice device);

        void setFlags(VkDescriptorPoolCreateFlagBits flags);
        void setMaxSets(uint32_t count);
        void setPoolSizes(std::vector<VkDescriptorPoolSize> poolSizes);

        VkDescriptorPool getHandler() const;

    private:
        VkDescriptorPoolCreateInfo mInfo{};
        VkDescriptorPool mHandler;
        std::vector<VkDescriptorPoolSize> mPoolSizes;
};

#endif