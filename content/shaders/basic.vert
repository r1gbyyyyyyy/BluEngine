#version 450
layout(set = 1, binding = 0, std140) uniform UniformBuffer {
    mat4 mvp;
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 0) out vec3 fragColor;

void main() {
    gl_Position = ubo.mvp * vec4(inPos, 1.0);
    fragColor = inColor;
}
