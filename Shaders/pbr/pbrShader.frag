#version 450 core

out vec4 FragColor;

in vec2 TexCoord;
in vec3 WorldPos;
in vec3 Normal;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

struct PointLight {
    vec3 position;
    vec3 color;
};

struct DirectionLight {
    vec3 direction;
    vec3 color;
};

struct SpotLight {
    vec3 position;
    vec3 color;
    vec3 direction;
    float inner_angle;
    float outer_angle;
};

uniform PointLight points[3];
uniform DirectionLight dir;
uniform SpotLight spot;

uniform vec3 cameraPos;

const float PI = 3.14159265359;
const float base_albedo = 0.04;
const float MAX_REFLECTION_LOD = 4.0;

vec3 ExtractNormal() {
    vec3 tan_normal = texture(normalMap, TexCoord).xyz * 2.0 - 1.0;

    vec3 world_deri_x = dFdx(WorldPos);
    vec3 world_deri_y = dFdy(WorldPos);
    vec2 tex_deri_x = dFdx(TexCoord);
    vec2 tex_deri_y = dFdy(TexCoord);

    vec3 N = normalize(Normal);
    vec3 T = normalize(world_deri_x * tex_deri_y.t - world_deri_y * tex_deri_x.t);
    vec3 B = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tan_normal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 CalculatePointRadiance(PointLight light) {
    float dist = length(light.position - WorldPos);
    return light.color * (1.0 / (dist * dist));
}

vec3 CalculateSpotRadiance(SpotLight light) {
    vec3 dir = normalize(light.position - WorldPos);
    float cosTheta = dot(dir, normalize(light.direction));
    float epsilon = light.inner_angle - light.outer_angle;
    float intensity = clamp((cosTheta - light.outer_angle) / epsilon, 0.0, 1.0);

    return intensity * light.color;
}

vec3 CalculatePBRRadiance(vec3 N, vec3 V, vec3 L, vec3 albedo, float metallic, float roughness, vec3 lighting, vec3 F0) {
    vec3 H = normalize(V + L);
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(V, H), 0.0), F0);

    vec3 nom = NDF * G * F;
    float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = nom / denom;

    vec3 kD = vec3(1.0) - F;
    kD *= (1.0 - metallic);

    float NdotL = max(dot(N, L), 0.0);
    return (kD * albedo / PI + specular) * lighting * NdotL;
}

void main() {
    vec3 albedo = pow(texture(albedoMap, TexCoord).rgb, vec3(2.2));
    float metallic = texture(metallicMap, TexCoord).r;
    float roughness = texture(roughnessMap, TexCoord).r;
    float ao = texture(aoMap, TexCoord).r;

    vec3 N = ExtractNormal();
    vec3 V = normalize((cameraPos - WorldPos));
    vec3 R = reflect(-V, N);

    vec3 F0 = vec3(base_albedo);
    F0 = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 3; i++) {
        vec3 L = normalize(points[i].position - WorldPos);
        Lo += CalculatePBRRadiance(N, V, L, albedo, metallic, roughness, CalculatePointRadiance(points[i]), F0);
    }
    vec3 L = normalize(spot.position - WorldPos);
    Lo += CalculatePBRRadiance(N, V, L, albedo, metallic, roughness, CalculateSpotRadiance(spot), F0);
    L = normalize(dir.direction);
    Lo += CalculatePBRRadiance(N, V, L, albedo, metallic, roughness, dir.color, F0);

    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD = 1.0 - F;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    vec3 prefilteredColor = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F0 * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;
    vec3 color = ambient + Lo;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}