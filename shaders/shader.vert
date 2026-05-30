#version 450

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec4 inColor;
layout(location = 0) out vec4 fragColor;

layout(push_constant) uniform Push {
    vec2 offset;
    vec2 screenRes;
} push;

void main() {
    vec2 finalPos = (inPos + push.offset) / (push.screenRes * 0.5);
    gl_Position = vec4(finalPos.x, -finalPos.y, 0.0, 1.0);
    fragColor = inColor;
}
