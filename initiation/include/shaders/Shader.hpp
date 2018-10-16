#ifndef SHADER
#define SHADER

#include <string>

#include <vulkan/vulkan.h>

class Shader {
    public:
        Shader();
        Shader(VkDevice device, std::string filename, VkShaderStageFlagBits shaderStage, std::string entryPointName);
        
        Shader(Shader& other) = delete;
        Shader& operator=(Shader& other) = delete;
        
        Shader(Shader&& other);
        Shader& operator=(Shader&& other);
        
        ~Shader();

        VkShaderModule createModule();
        VkPipelineShaderStageCreateInfo getCreateInfo();
        void clean();

    private:
        VkDevice mDevice;
        VkShaderModule mModule;
        VkShaderStageFlagBits mStage;
        VkShaderModuleCreateInfo mModuleInfo{};
        std::string mEntryPointName;
        std::string mFilename;
        std::vector<char> mCode;

        bool mCreated{false};
};

#endif