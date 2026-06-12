#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 fragColor;
layout(location = 2) in flat uint inRenderType;

layout(location = 0) out vec4 outColor;

layout(std430, binding = 0) buffer TextureBuffer {
    uint width;
    uint height;
    uint pixels[];
};

void main() {
    if (inRenderType == 0) {
        outColor = fragColor;
    } else {
        uint x = uint(fragUV.x * float(width));
        uint y = uint(fragUV.y * float(height));

        if (x >= width) x = width - 1;
        if (y >= height) y = height - 1;

        uint idx = y * width + x;
        uint rawColor = pixels[idx];

        outColor = unpackUnorm4x8(rawColor);
    }
}
