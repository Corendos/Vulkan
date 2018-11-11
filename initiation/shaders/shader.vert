#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragNormal;
layout(location = 2) out vec3 fragColor;
layout(location = 3) out vec2 fragTexCoord;
layout(location = 4) out vec3 cameraPosition;

layout(set = 1, binding = 0) uniform RenderInfo {
    mat4 view;
    mat4 proj;
    vec4 position;
    vec4 lightPosition;
} renderInfo;

layout(set = 0, binding = 1) uniform ModelMatrix {
    mat4 matrix;
} model;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = renderInfo.proj * renderInfo.view * model.matrix * vec4(inPosition, 1.0);
    fragPosition = inPosition;
    fragNormal = inNormal;
    fragColor = vec3(1.0);
    fragTexCoord = inTexCoord;
    cameraPosition = renderInfo.lightPosition.xyz;
}