#ifndef MESHMANAGER
#define MESHMANAGER

#include <vector>
#include <array>
#include <map>

#include <vulkan/vulkan.h>

#include "renderer/Mesh.hpp"

struct MeshData {
    VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
    uint32_t uniformBufferDynamicOffset{0};
    bool free{true};
};

class MeshManager {
    public:
        MeshManager();
        MeshManager(const MeshManager& other) = delete;
        MeshManager(const MeshManager&& other) = delete;

        void operator=(const MeshManager& other) = delete;
        void operator=(const MeshManager&& other) = delete;

        void create(VulkanContext& context);
        void destroy();

        void addMesh(Mesh& mesh);
        void removeMesh(Mesh& mesh);

        void updateUniformBuffer();
        void render(VkCommandBuffer commandBuffer, VkPipelineLayout layout);
        VkDescriptorSetLayout getDescriptorSetLayout() const;

        static constexpr size_t MaximumMeshCount{1024};
    private:
        struct {
            VkBuffer vertexBuffer;
            uint32_t vertexBufferSize{0};
            VkBuffer indexBuffer;
            uint32_t indexBufferSize{0};
            VkBuffer modelTransformBuffer;
            uint32_t modelTransformBufferSize{0};
            VkDescriptorSetLayout descriptorSetLayout;
            std::array<MeshData, MaximumMeshCount> meshDataPool;
            std::map<Mesh*, MeshData*> meshDataBinding;
        } mRenderData;

        VulkanContext* mContext;

        std::vector<Mesh*> mMeshes;

        void createDescriptorSetLayout();
        void allocateUniformBuffer();
        void allocateDescriptorSets();
        void updateStaticBuffers();
        void updateDescriptorSet(Mesh& mesh, VkDescriptorSet& descriptorSet);
};

#endif