#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 cameraPosition;
layout(location = 4) in vec3 fragPosition;

layout(set = 0, binding = 0) uniform sampler2D texSampler;

void main() {
    float coef = max(dot(normalize(cameraPosition), normalize(fragNormal)), 0.3);
    outColor = coef * texture(texSampler, fragTexCoord);
}