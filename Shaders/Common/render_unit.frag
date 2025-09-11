#version 450 core

out vec4 FragColor;

in vec2 texCoord;

layout (binding = 0) uniform sampler2D in_texture;

// 默认恰有两位置位
uniform uint channels;

void main()
{
    vec4 pixel_color = texture(in_texture, texCoord);
    float temp[4];

    temp[0] = pixel_color.r;
    temp[1] = pixel_color.g;
    temp[2] = pixel_color.b;
    temp[3] = pixel_color.a;

    uint count = 0;
    for (int i = 3; i >= 0; i--)
    {
        if ((channels & (1 << i)) != 0)
        {
            temp[count] = temp[3 - i];
            count++;
        }
    }

    temp[2] = sqrt(clamp(1.0 - temp[0] * temp[0] - temp[1] * temp[1], 0.0, 1.0));
    
    FragColor = vec4(temp[0], temp[1], temp[2], 1.0);
}