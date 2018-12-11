#ifndef MESHMANAGER
#define MESHMANAGER

#include <vector>
#include <future>
#include <array>
#include <map>
#include <atomic>

#include <vulkan/vulkan.h>

#include "renderer/mesh/Mesh.hpp"

struct MeshData {
    VkDescriptorSet descriptorSet{VK_NULL_HANDLE};
    uint32_t uniformBufferDynamicOffset{0};
    bool free{true};
};

struct RenderBuffers {
    VkBuffer vertexBuffer;
    VkBuffer indexBuffer;
    uint32_t vertexBufferSize{0};
    uint32_t vertexBufferSizeInBytes{0};
    uint32_t indexBufferSize{0};
    uint32_t indexBufferSizeInBytes{0};
    bool needUpdate{false};
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

        void setImageCount(uint32_t count);

        bool update(uint32_t imageIndex);
        void render(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint32_t imageIndex);
        VkDescriptorSetLayout getDescriptorSetLayout() const;

        static constexpr size_t MaximumMeshCount{1024};
    private:
        struct {
            std::vector<RenderBuffers> renderBuffers;
            RenderBuffers stagingBuffers;

            VkBuffer modelTransformBuffer;
            uint32_t modelTransformBufferSize{0};
            VkDescriptorSetLayout descriptorSetLayout;
            std::array<MeshData, MaximumMeshCount> meshDataPool;
            std::map<Mesh*, MeshData*> meshDataBinding;
        } mRenderData;
        std::atomic_bool mStagingUpdated{false};
        std::future<void> mStagingUpdateResult;

        VulkanContext* mContext;

        std::vector<Mesh*> mMeshes;

        void createDescriptorSetLayout();
        void allocateUniformBuffer();
        void allocateDescriptorSets();
        void updateStagingBuffers();
        void updateDescriptorSet(Mesh& mesh, MeshData& meshData);
        void updateUniformBuffer();
        bool updateStaticBuffers(uint32_t imageIndex);
};

#endif