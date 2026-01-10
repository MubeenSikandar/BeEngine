#pragma once

#include <string>

namespace BeEngine {

// ============================================================
// UNLIT SHADER - No lighting, just texture and color
// ============================================================
inline const std::string UnlitVertex = R"(
    #version 410 core

    layout(location = 0) in vec3 a_Position;
    layout(location = 1) in vec3 a_Normal;
    layout(location = 2) in vec4 a_Tangent;
    layout(location = 3) in vec2 a_TexCoord;
    layout(location = 4) in vec4 a_Color;

    uniform mat4 u_ViewProjection;
    uniform mat4 u_Model;

    out vec2 v_TexCoord;
    out vec4 v_Color;

    void main() {
        v_TexCoord = a_TexCoord;
        v_Color = a_Color;
        gl_Position = u_ViewProjection * u_Model * vec4(a_Position, 1.0);
    }
    )";

inline const std::string UnlitFragment = R"(
    #version 410 core

    in vec2 v_TexCoord;
    in vec4 v_Color;

    uniform sampler2D u_AlbedoMap;
    uniform vec4 u_Color;
    uniform bool u_UseAlbedoMap;
    uniform float u_AlphaCutoff;

    layout(location = 0) out vec4 o_Color;

    void main() {
        vec4 albedo = u_UseAlbedoMap ? texture(u_AlbedoMap, v_TexCoord) : vec4(1.0);
        vec4 finalColor = albedo * u_Color * v_Color;

        // Alpha cutoff for transparency
        if (finalColor.a < u_AlphaCutoff) {
            discard;
        }

        o_Color = finalColor;
    }
    )";

// ============================================================
// PHONG SHADER - Classic ambient + diffuse + specular
// ============================================================
inline const std::string PhongVertex = R"(
    #version 410 core

    layout(location = 0) in vec3 a_Position;
    layout(location = 1) in vec3 a_Normal;
    layout(location = 2) in vec4 a_Tangent;
    layout(location = 3) in vec2 a_TexCoord;
    layout(location = 4) in vec4 a_Color;

    uniform mat4 u_ViewProjection;
    uniform mat4 u_Model;
    uniform mat3 u_NormalMatrix;

    out vec3 v_WorldPos;
    out vec3 v_Normal;
    out vec2 v_TexCoord;
    out vec4 v_Color;

    void main() {
        vec4 worldPos = u_Model * vec4(a_Position, 1.0);
        v_WorldPos = worldPos.xyz;
        v_Normal = u_NormalMatrix * a_Normal;
        v_TexCoord = a_TexCoord;
        v_Color = a_Color;
        gl_Position = u_ViewProjection * worldPos;
    }
    )";

inline const std::string PhongFragment = R"(
    #version 410 core

    in vec3 v_WorldPos;
    in vec3 v_Normal;
    in vec2 v_TexCoord;
    in vec4 v_Color;

    // Material properties
    uniform sampler2D u_DiffuseMap;
    uniform sampler2D u_SpecularMap;
    uniform vec4 u_DiffuseColor;
    uniform vec4 u_SpecularColor;
    uniform float u_Shininess;
    uniform bool u_UseDiffuseMap;
    uniform bool u_UseSpecularMap;

    // Lighting
    uniform vec3 u_CameraPos;
    uniform vec3 u_AmbientLight;

    // Directional Light
    uniform vec3 u_DirLightDirection;
    uniform vec3 u_DirLightColor;
    uniform float u_DirLightIntensity;

    // Point Lights (up to 4)
    #define MAX_POINT_LIGHTS 4
    uniform int u_PointLightCount;
    uniform vec3 u_PointLightPositions[MAX_POINT_LIGHTS];
    uniform vec3 u_PointLightColors[MAX_POINT_LIGHTS];
    uniform float u_PointLightIntensities[MAX_POINT_LIGHTS];
    uniform float u_PointLightRanges[MAX_POINT_LIGHTS];

    layout(location = 0) out vec4 o_Color;

    vec3 CalcDirLight(vec3 normal, vec3 viewDir, vec3 diffuseColor, vec3 specularColor) {
        vec3 lightDir = normalize(-u_DirLightDirection);

        // Diffuse
        float diff = max(dot(normal, lightDir), 0.0);

        // Specular (Blinn-Phong)
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Shininess);

        vec3 diffuse = u_DirLightColor * diff * diffuseColor * u_DirLightIntensity;
        vec3 specular = u_DirLightColor * spec * specularColor * u_DirLightIntensity;

        return diffuse + specular;
    }

    vec3 CalcPointLight(int index, vec3 normal, vec3 viewDir, vec3 fragPos,
                        vec3 diffuseColor, vec3 specularColor) {
        vec3 lightPos = u_PointLightPositions[index];
        vec3 lightColor = u_PointLightColors[index];
        float intensity = u_PointLightIntensities[index];
        float range = u_PointLightRanges[index];

        vec3 lightDir = normalize(lightPos - fragPos);
        float distance = length(lightPos - fragPos);

        // Attenuation
        float attenuation = 1.0 / (1.0 + (distance / range) * (distance / range));
        attenuation = max(attenuation, 0.0);

        // Diffuse
        float diff = max(dot(normal, lightDir), 0.0);

        // Specular (Blinn-Phong)
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), u_Shininess);

        vec3 diffuse = lightColor * diff * diffuseColor * intensity * attenuation;
        vec3 specular = lightColor * spec * specularColor * intensity * attenuation;

        return diffuse + specular;
    }

    void main() {
        vec3 normal = normalize(v_Normal);
        vec3 viewDir = normalize(u_CameraPos - v_WorldPos);

        // Sample textures
        vec3 diffuseColor = u_UseDiffuseMap
            ? texture(u_DiffuseMap, v_TexCoord).rgb * u_DiffuseColor.rgb
            : u_DiffuseColor.rgb;
        diffuseColor *= v_Color.rgb;

        vec3 specularColor = u_UseSpecularMap
            ? texture(u_SpecularMap, v_TexCoord).rgb * u_SpecularColor.rgb
            : u_SpecularColor.rgb;

        // Ambient
        vec3 ambient = u_AmbientLight * diffuseColor;

        // Directional light
        vec3 result = ambient + CalcDirLight(normal, viewDir, diffuseColor, specularColor);

        // Point lights
        for (int i = 0; i < u_PointLightCount && i < MAX_POINT_LIGHTS; i++) {
            result += CalcPointLight(i, normal, viewDir, v_WorldPos, diffuseColor, specularColor);
        }

        o_Color = vec4(result, u_DiffuseColor.a);
    }
    )";

// ============================================================
// PBR SHADER - Physically-Based Rendering (Metallic/Roughness)
// ============================================================
inline const std::string PBRVertex = R"(
    #version 410 core

    layout(location = 0) in vec3 a_Position;
    layout(location = 1) in vec3 a_Normal;
    layout(location = 2) in vec4 a_Tangent;
    layout(location = 3) in vec2 a_TexCoord;
    layout(location = 4) in vec4 a_Color;

    uniform mat4 u_ViewProjection;
    uniform mat4 u_Model;
    uniform mat3 u_NormalMatrix;

    out vec3 v_WorldPos;
    out vec3 v_Normal;
    out vec2 v_TexCoord;
    out vec4 v_Color;
    out mat3 v_TBN;

    void main() {
        vec4 worldPos = u_Model * vec4(a_Position, 1.0);
        v_WorldPos = worldPos.xyz;
        v_Normal = u_NormalMatrix * a_Normal;
        v_TexCoord = a_TexCoord;
        v_Color = a_Color;

        // Calculate TBN matrix for normal mapping
        vec3 T = normalize(u_NormalMatrix * a_Tangent.xyz);
        vec3 N = normalize(v_Normal);
        T = normalize(T - dot(T, N) * N); // Re-orthogonalize
        vec3 B = cross(N, T) * a_Tangent.w;
        v_TBN = mat3(T, B, N);

        gl_Position = u_ViewProjection * worldPos;
    }
    )";

inline const std::string PBRFragment = R"(
    #version 410 core

    in vec3 v_WorldPos;
    in vec3 v_Normal;
    in vec2 v_TexCoord;
    in vec4 v_Color;
    in mat3 v_TBN;

    // Material textures
    uniform sampler2D u_AlbedoMap;
    uniform sampler2D u_NormalMap;
    uniform sampler2D u_MetallicMap;
    uniform sampler2D u_RoughnessMap;
    uniform sampler2D u_AOMap;
    uniform sampler2D u_EmissiveMap;

    // Material properties
    uniform vec4 u_AlbedoColor;
    uniform float u_Metallic;
    uniform float u_Roughness;
    uniform float u_AO;
    uniform vec3 u_EmissiveColor;
    uniform float u_EmissiveIntensity;

    // Texture usage flags
    uniform bool u_UseAlbedoMap;
    uniform bool u_UseNormalMap;
    uniform bool u_UseMetallicMap;
    uniform bool u_UseRoughnessMap;
    uniform bool u_UseAOMap;
    uniform bool u_UseEmissiveMap;

    // Lighting
    uniform vec3 u_CameraPos;
    uniform vec3 u_AmbientLight;

    // Directional Light
    uniform vec3 u_DirLightDirection;
    uniform vec3 u_DirLightColor;
    uniform float u_DirLightIntensity;

    // Point Lights
    #define MAX_POINT_LIGHTS 4
    uniform int u_PointLightCount;
    uniform vec3 u_PointLightPositions[MAX_POINT_LIGHTS];
    uniform vec3 u_PointLightColors[MAX_POINT_LIGHTS];
    uniform float u_PointLightIntensities[MAX_POINT_LIGHTS];
    uniform float u_PointLightRanges[MAX_POINT_LIGHTS];

    layout(location = 0) out vec4 o_Color;

    const float PI = 3.14159265359;

    // Normal Distribution Function (GGX/Trowbridge-Reitz)
    float DistributionGGX(vec3 N, vec3 H, float roughness) {
        float a = roughness * roughness;
        float a2 = a * a;
        float NdotH = max(dot(N, H), 0.0);
        float NdotH2 = NdotH * NdotH;

        float nom = a2;
        float denom = (NdotH2 * (a2 - 1.0) + 1.0);
        denom = PI * denom * denom;

        return nom / max(denom, 0.0001);
    }

    // Geometry Function (Schlick-GGX)
    float GeometrySchlickGGX(float NdotV, float roughness) {
        float r = roughness + 1.0;
        float k = (r * r) / 8.0;

        float nom = NdotV;
        float denom = NdotV * (1.0 - k) + k;

        return nom / max(denom, 0.0001);
    }

    // Geometry Function (Smith)
    float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
        float NdotV = max(dot(N, V), 0.0);
        float NdotL = max(dot(N, L), 0.0);
        float ggx2 = GeometrySchlickGGX(NdotV, roughness);
        float ggx1 = GeometrySchlickGGX(NdotL, roughness);

        return ggx1 * ggx2;
    }

    // Fresnel (Schlick approximation)
    vec3 FresnelSchlick(float cosTheta, vec3 F0) {
        return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
    }

    vec3 CalcPBRLight(vec3 lightDir, vec3 lightColor, float lightIntensity,
                      vec3 N, vec3 V, vec3 albedo, float metallic, float roughness, vec3 F0) {
        vec3 L = normalize(lightDir);
        vec3 H = normalize(V + L);

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - metallic; // Metals have no diffuse

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 specular = numerator / denominator;

        float NdotL = max(dot(N, L), 0.0);

        return (kD * albedo / PI + specular) * lightColor * lightIntensity * NdotL;
    }

    void main() {
        // Sample material properties
        vec3 albedo = u_UseAlbedoMap
            ? texture(u_AlbedoMap, v_TexCoord).rgb * u_AlbedoColor.rgb
            : u_AlbedoColor.rgb;
        albedo *= v_Color.rgb;

        float metallic = u_UseMetallicMap
            ? texture(u_MetallicMap, v_TexCoord).r * u_Metallic
            : u_Metallic;

        float roughness = u_UseRoughnessMap
            ? texture(u_RoughnessMap, v_TexCoord).r * u_Roughness
            : u_Roughness;
        roughness = max(roughness, 0.04); // Prevent divide by zero

        float ao = u_UseAOMap
            ? texture(u_AOMap, v_TexCoord).r * u_AO
            : u_AO;

        vec3 emissive = u_UseEmissiveMap
            ? texture(u_EmissiveMap, v_TexCoord).rgb * u_EmissiveColor * u_EmissiveIntensity
            : u_EmissiveColor * u_EmissiveIntensity;

        // Normal mapping
        vec3 N;
        if (u_UseNormalMap) {
            vec3 normalMap = texture(u_NormalMap, v_TexCoord).rgb * 2.0 - 1.0;
            N = normalize(v_TBN * normalMap);
        } else {
            N = normalize(v_Normal);
        }

        vec3 V = normalize(u_CameraPos - v_WorldPos);

        // Calculate F0 (surface reflection at zero incidence)
        vec3 F0 = vec3(0.04); // Dielectric default
        F0 = mix(F0, albedo, metallic); // Metals use albedo as F0

        // Ambient (simple IBL approximation)
        vec3 ambient = u_AmbientLight * albedo * ao;

        // Directional light
        vec3 Lo = CalcPBRLight(-u_DirLightDirection, u_DirLightColor, u_DirLightIntensity,
                               N, V, albedo, metallic, roughness, F0);

        // Point lights
        for (int i = 0; i < u_PointLightCount && i < MAX_POINT_LIGHTS; i++) {
            vec3 lightDir = u_PointLightPositions[i] - v_WorldPos;
            float distance = length(lightDir);
            float range = u_PointLightRanges[i];
            float attenuation = 1.0 / (1.0 + (distance / range) * (distance / range));

            Lo += CalcPBRLight(lightDir, u_PointLightColors[i],
                              u_PointLightIntensities[i] * attenuation,
                              N, V, albedo, metallic, roughness, F0);
        }

        vec3 color = ambient + Lo + emissive;

        // HDR tonemapping (simple Reinhard)
        color = color / (color + vec3(1.0));

        // Gamma correction
        color = pow(color, vec3(1.0 / 2.2));

        o_Color = vec4(color, u_AlbedoColor.a);
    }
    )";
} // namespace BeEngine
