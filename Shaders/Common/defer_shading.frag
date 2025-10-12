#version 450 core

#define MAX_LIGHT_COUNT 16
#define PI 3.1415926535

#define LIGHT_TYPE_DIRECTIONAL  0
#define LIGHT_TYPE_POINT        1
#define LIGHT_TYPE_SPOT         2
#define LIGHT_TYPE_RECT         3

#define LIGHT_STATUS_INVALID    0xFFFFFFFF

layout (location = 0) out vec4 FragColor;

layout (location = 0) in vec2 texCoord;

layout (binding = 0) uniform sampler2D position_anisotrophy_tex;
layout (binding = 1) uniform sampler2D albedo_specular_tex;
layout (binding = 2) uniform sampler2D normal_metallic_roughness_tex;

uniform vec3 cameraPos;

struct LightCommon
{
    vec3 position;
    uint flags;
    vec3 color;
    float intensity;
    vec4 customs;
};

layout (std140, binding = 0) uniform LightBuffer
{
    LightCommon lights[MAX_LIGHT_COUNT];
};

// 位置和法线均在世界空间中
struct SurfaceData
{
    vec3 position;
    vec3 normal;
    vec3 albedo;
    float specular;
    float anisotrophy;
    float metallic;
    float roughness;
};

void PrepareSurfaceData(inout SurfaceData data)
{
    vec4 position_anisotrophy = texture(position_anisotrophy_tex, texCoord);
    data.position = position_anisotrophy.xyz;
    data.anisotrophy = position_anisotrophy.a;
    
    vec4 albedo_specular = texture(albedo_specular_tex, texCoord);
    data.albedo = albedo_specular.xyz;
    data.specular = albedo_specular.a;

    vec4 normal_metallic_roughness = texture(normal_metallic_roughness_tex, texCoord);
    data.metallic = normal_metallic_roughness.z;
    data.roughness = normal_metallic_roughness.a;
    data.normal = vec3(normal_metallic_roughness.xy, sqrt(1.0 - dot(normal_metallic_roughness.xy, normal_metallic_roughness.xy)));

    return;
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 Lighting_Common(vec3 V, vec3 L, vec3 H, vec3 radiance, SurfaceData surfaceData)
{
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, surfaceData.albedo, surfaceData.metallic);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 N = normalize(surfaceData.normal);

    float NDF = DistributionGGX(N, H, surfaceData.roughness);       
    float G = GeometrySmith(N, V, L, surfaceData.roughness);  

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0)  + 0.0001;
    vec3 specular = numerator / denominator;  

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - surfaceData.metallic;

    float NdotL = max(dot(N, L), 0.0);        
    return (kD * surfaceData.albedo / PI + specular) * radiance * NdotL;
}

vec3 Directional_Lighting(uint i, vec3 V, SurfaceData surfaceData)
{
    vec3 L = -normalize(lights[i].position);
    vec3 H = normalize(V + L);

    vec3 radiance = lights[i].color * lights[i].intensity;

    return Lighting_Common(V, L, H, radiance, surfaceData);
}

vec3 PointSpot_Lighting(uint i, vec3 V, bool is_spot, SurfaceData surfaceData)
{
    vec3 L = normalize(lights[i].position - surfaceData.position);
    vec3 H = normalize(V + L);

    float dist = length(lights[i].position - surfaceData.position);
    float attenuation = 1.0 / (dist * dist);
    if (is_spot)
    {
        vec3 spot_direction = vec3(lights[i].customs.xy, sign(lights[i].intensity) * sqrt(1.0 - dot(lights[i].customs.xy, lights[i].customs.xy)));
        float cos_angle = dot(L, -spot_direction);
        attenuation *= clamp((cos_angle - lights[i].customs.z) * lights[i].customs.w, 0.0, 1.0);
    }
    vec3 radiance = lights[i].color * abs(lights[i].intensity) * attenuation;

    return Lighting_Common(V, L, H, radiance, surfaceData);
}

void main()
{
    SurfaceData surfaceData;
    PrepareSurfaceData(surfaceData);

    vec3 V = normalize(cameraPos - surfaceData.position);

    vec3 result = vec3(0.0);
    for (uint i = 0; i < MAX_LIGHT_COUNT; i++)
    {
        if (lights[i].flags == LIGHT_STATUS_INVALID)
        {
            continue;
        }

        switch (lights[i].flags)
        {
            case LIGHT_TYPE_DIRECTIONAL :
            {
                result += Directional_Lighting(i, V, surfaceData);
                break;
            }
            case LIGHT_TYPE_POINT :
            case LIGHT_TYPE_SPOT :
            {
                result += PointSpot_Lighting(i, V, lights[i].flags == LIGHT_TYPE_SPOT, surfaceData);
                break;
            }
            case LIGHT_TYPE_RECT :
            {
                // TODO
                break;
            }
            default :
            {
                break;
            }
        }
    }

    vec3 ambient = vec3(0.03) * surfaceData.albedo;

    FragColor = vec4(ambient + result, 1.0);
}