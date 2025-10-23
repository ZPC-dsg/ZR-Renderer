#version 450 core

#define MAX_FILTER_SAMPLES 32
#define PACKED_MAX_FILTER_SAMPLES ((MAX_FILTER_SAMPLES + 1) / 2) 

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 texCoord;
layout (location = 1) in vec4 sample_offsets[PACKED_MAX_FILTER_SAMPLES];

layout (binding = 0) uniform sampler2D input_texture;
layout (binding = 1) uniform sampler2D additive_texture;

uniform vec4 sample_weights[MAX_FILTER_SAMPLES];
uniform uint sample_count;
uniform bool use_additive;

void main()
{
    vec3 final_color = vec3(0.0);

    for (int i = 0; i < sample_count - 1; i += 2)
    {
        final_color += texture(input_texture, sample_offsets[i / 2].xy).rgb * sample_weights[i].rgb;
        final_color += texture(input_texture, sample_offsets[i / 2].zw).rgb * sample_weights[i + 1].rgb;
    }

    if ((sample_count & 1) > 0)
    {
        final_color += texture(input_texture, sample_offsets[(sample_count - 1) / 2].xy).rgb * sample_weights[sample_count - 1].rgb;
    }

    if (use_additive)
    {
        final_color += texture(additive_texture, texCoord).rgb;
    }

    FragColor = vec4(final_color, 1.0);
}