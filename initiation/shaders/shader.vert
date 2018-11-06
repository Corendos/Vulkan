#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 cameraPosition;
layout(location = 4) out vec3 fragPosition;

layout(set = 1, binding = 0) uniform CameraInfo {
    mat4 view;
    mat4 proj;
    vec4 position;
    vec4 lightPosition;
} cameraInfo;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = cameraInfo.proj * cameraInfo.view * vec4(inPosition, 1.0);
    fragPosition = inPosition;
    fragTexCoord = inTexCoord;
    fragColor = vec3(1.0);
    fragNormal = inNormal;
    cameraPosition = cameraInfo.lightPosition.xyz;
}