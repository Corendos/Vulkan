#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec3 outColor;
layout(location = 3) out vec2 outTexCoord;
layout(location = 4) out vec3 outLightPosition;
layout(location = 5) out mat4 outModelMatrix;
layout(location = 9) out mat4 outViewMatrix;

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

    outPosition = inPosition;
    outNormal = vec4(inNormal, 0.0);
    outColor = vec3(1.0);
    outTexCoord = inTexCoord;
    outLightPosition = renderInfo.lightPosition.xyz;
    outModelMatrix = model.matrix;
    outViewMatrix = renderInfo.view;
}