#version 450 core

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 texCoord;

layout (binding = 0) uniform sampler2D input_texture;

void main()
{
    uvec2 input_size = textureSize(input_texture, 0);
    uvec2 ouput_size = (input_size + uvec2(1, 1)) / 2;

    vec3 outColor = vec3(0.0);

    vec2 uv[4];
    uv[0] = texCoord + vec2(-1.0, 1.0) / vec2(input_size);
    uv[1] = texCoord + vec2(1.0, 1.0) / vec2(input_size);
    uv[2] = texCoord + vec2(1.0, -1.0) / vec2(input_size);
    uv[3] = texCoord + vec2(-1.0, -1.0) / vec2(input_size);

    for (int i = 0; i < 4; i++)
    {
        outColor += texture(input_texture, uv[i]).rgb;
    }

    FragColor = vec4(outColor * 0.25, 1.0);
}