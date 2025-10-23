#version 450 core

#define MAX_FILTER_SAMPLES 32
#define PACKED_MAX_FILTER_SAMPLES ((MAX_FILTER_SAMPLES + 1) / 2) 

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

layout (location = 0) out vec2 texCoord;
layout (location = 1) out vec4 sample_offsets[PACKED_MAX_FILTER_SAMPLES];

uniform vec4 offsets[PACKED_MAX_FILTER_SAMPLES];
uniform uint sample_count;

void main()
{
    texCoord = aTexCoord;

    uint packed_sample_count = (sample_count + 1) / 2;
    for (uint i = 0; i < packed_sample_count; i++)
    {
        sample_offsets[i] = vec4(aTexCoord, aTexCoord) + offsets[i];
    }

    gl_Position = vec4(aPos, 1.0);
}