#ifndef OBJECT
#define OBJECT

#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

#include "vulkan/Vertex.hpp"
#include "resources/Texture.hpp"

class Object {
    public:
        Object(Object& other);
        Object(Object&& other) = default;
        Object(std::vector<Vertex> vertices, std::vector<uint32_t> indices);
        Object(std::vector<Vertex>&& vertices, std::vector<uint32_t>&& indices);

        Object& operator=(Object& other);
        Object& operator=(Object&& other) = default;

        uint32_t getVertexCount() const;
        uint32_t getIndexCount() const;

        std::vector<Vertex>& getVertices();
        std::vector<uint32_t>& getIndices();
        Texture& getTexture();
        VkDescriptorSet getDescriptorSet() const;
        void setDescriptorSet(VkDescriptorSet descriptorSet);
        void setTexture(Texture& texture);

        static Object temp(glm::vec3 position);

    private:
        std::vector<Vertex> mVertices;
        std::vector<uint32_t> mIndices;
        unsigned long mUniqueId;

        Texture* mTexture;

        VkDescriptorSet mDescriptorSet;

        static unsigned long nextId;
};

#endif