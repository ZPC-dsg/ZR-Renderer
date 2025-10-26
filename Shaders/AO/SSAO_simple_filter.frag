#version 450 core

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 texCoord;

layout (binding = 0) uniform sampler2D ao_texture;

void main()
{
    vec2 texel_size = vec2(1.0) / vec2(textureSize(ao_texture, 0));
    float res = 0.0;
    for (int i = -2; i < 2; i++)
    {
        for (int j = -2; j < 2; j++)
        {
            res += texture(ao_texture, texCoord + vec2(i, j) * texel_size).r;
        }
    }
    res /= 16.0;

    FragColor = vec4(vec3(res), 1.0);
}