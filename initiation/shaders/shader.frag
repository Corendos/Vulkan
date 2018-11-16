#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec3 outNormal;

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 fragColor;
layout(location = 3) in vec2 fragTexCoord;
layout(location = 4) in vec3 lightPosition;

layout(set = 0, binding = 0) uniform sampler2D texSampler;

void main() {
    vec3 toCamera = lightPosition - fragPosition;
    float coef = max(dot(normalize(toCamera), normalize(fragNormal)), 0.3);
    outColor = coef * texture(texSampler, fragTexCoord);
    outNormal = fragNormal;
}