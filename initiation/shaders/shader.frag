#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec4 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inTexCoord;
layout(location = 4) in vec3 inLightPosition;
layout(location = 5) in mat4 inModelMatrix;
layout(location = 9) in mat4 inViewMatrix;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec3 outNormal;

layout(set = 0, binding = 0) uniform sampler2D texSampler;

void main() {
    vec3 toCamera = inLightPosition - inPosition;
    float coef = max(dot(normalize(toCamera), normalize(inNormal.xyz)), 0.3);
    outColor = coef * texture(texSampler, inTexCoord);
    outNormal = (inModelMatrix * inNormal).xyz / 2.0 + vec3(0.5, 0.5, 0.5);
}