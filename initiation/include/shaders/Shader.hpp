#ifndef SHADER
#define SHADER

#include <string>

#include <vulkan/vulkan.h>

class Shader {
    public:
        Shader();
        Shader(std::string filename, VkShaderStageFlagBits shaderStage, std::string entryPointName);
        
        Shader(Shader& other) = delete;
        Shader& operator=(Shader& other) = delete;
        
        Shader(Shader&& other);
        Shader& operator=(Shader&& other);
        
        ~Shader();

        void create(VkDevice device);
        void destroy(VkDevice device);

        VkShaderModule createModule();
        VkPipelineShaderStageCreateInfo getCreateInfo() const;

    private:
        VkShaderModule mModule;
        VkShaderStageFlagBits mStage;
        VkShaderModuleCreateInfo mModuleInfo{};
        std::string mEntryPointName;
        std::string mFilename;
        std::vector<char> mCode;

        bool mCreated{false};
};

#endif