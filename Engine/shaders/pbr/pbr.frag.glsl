#version 450

layout(std140, binding = 0) uniform CameraShaderState
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
    mat4 modelMatrix;

    vec3 cameraPosition;// padded to 16 bytes
};

layout(std140, binding = 1) uniform MaterialShaderState {

// Contains flags for which textures exist
// bit (1 << 0): albedo
// bit (1 << 1): normal
// bit (1 << 2): rma
// if the respective bit is set, the respective uniform sampler can be used safely.
// No garauntee is made that the sampler is bound to the correct texture unit, if the respective texture is stated
// to not exist by these flags.
    int texturePresentStates;

    vec4 albedoFactor;
    float roughnessFactor;
    float metalnessFactor;
    float ambientOcclusionFactor;
    float normalScale;
};

layout(binding = 2) uniform sampler2D albedoSampler;
layout(binding = 3) uniform sampler2D normalSampler;
layout(binding = 4) uniform sampler2D rmaSampler;

#define MAX_POINT_LIGHTS 10
#define M_PI 3.1415926535897932384626433832795

layout(std140, binding = 4) uniform LightShaderState
{
    vec3 pointLightPositions[MAX_POINT_LIGHTS];// each element padded to 16 bytes
    vec4 pointLightColors[MAX_POINT_LIGHTS];

    int numPointLights;// placed at the end for spooky alignment reasons
};

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 vTexCoord;
//layout(location = 2) in vec4 vColor;
layout(location = 3) in mat3 TBN;

layout(location = 0) out vec4 fragColor;

vec3 FresnelSchlick(float cosTheta, vec3 F0);

float DistributionGGX(vec3 normal, vec3 halfVector, float roughness);

float GeometrySchlickGGX(float NdotV, float roughness);

float GeometrySmith(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness);

void main()
{
    vec4 samplerAlbedo = texture(albedoSampler, vTexCoord);

    float alpha = samplerAlbedo.a * albedoFactor.a;

    vec3 albedo;
    if ((texturePresentStates & (1 << 0)) != 0) {
        albedo = samplerAlbedo.rgb * albedoFactor.rgb;
    } else {
        albedo = albedoFactor.rgb;
    }

    vec3 samplerNormal;
    if ((texturePresentStates & (1 << 1)) != 0) {
        samplerNormal = texture(normalSampler, vTexCoord).xyz;
    } else {
        samplerNormal = vec3(0.0);
    }
    samplerNormal = normalize(samplerNormal * 2.0 - 1.0);

    vec3 rma;
    if ((texturePresentStates & (1 << 2)) != 0){
        rma = texture(rmaSampler, vTexCoord).rgb;
    } else {
        rma = vec3(1.0);
    }

    float roughness = rma.x;
    float metallic = rma.y;
    float ao = rma.z;

    vec3 N = normalize(TBN * samplerNormal);
    vec3 V = normalize(cameraPosition - vPos);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < 4; ++i)
    {
        // calculate per-light radiance
        vec3 L = normalize(pointLightPositions[i] - vPos);
        vec3 H = normalize(V + L);
        float distance    = length(pointLightPositions[i] - vPos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = pointLightColors[i].rgb * attenuation * pointLightColors[i].a;

        // cook-torrance brdf
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = FresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic;

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular     = numerator / denominator;

        // add to outgoing radiance Lo
        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * albedo / M_PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    fragColor = vec4(color, alpha);
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
    denom = M_PI * denom * denom;

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