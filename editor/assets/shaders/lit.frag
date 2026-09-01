#version 460 core

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float radius;
};

#define MAX_POINT_LIGHTS 8

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

out vec4 o_Color;

uniform sampler2D u_AlbedoTexture;
uniform vec3 u_AlbedoColor;
uniform vec3 u_ViewPos;

uniform DirectionalLight u_DirLight;
uniform int u_PointLightCount;
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];

vec3 computeDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 halfway = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfway), 0.0), 32.0);

    vec3 diffuse = diff * light.color * light.intensity * albedo;
    vec3 specular = spec * light.color * light.intensity * 0.3;
    return diffuse + specular;
}

vec3 computePointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo) {
    vec3 lightDir = normalize(light.position - fragPos);
    float dist = length(light.position - fragPos);
    float attenuation = clamp(1.0 - (dist / light.radius), 0.0, 1.0);
    attenuation *= attenuation;

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 halfway = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfway), 0.0), 32.0);

    vec3 diffuse = diff * light.color * light.intensity * albedo;
    vec3 specular = spec * light.color * light.intensity * 0.3;
    return (diffuse + specular) * attenuation;
}

void main() {
    vec3 albedo = texture(u_AlbedoTexture, v_TexCoord).rgb * u_AlbedoColor;
    vec3 normal = normalize(v_Normal);
    vec3 viewDir = normalize(u_ViewPos - v_FragPos);

    vec3 ambient = 0.08 * albedo;
    vec3 result = ambient + computeDirLight(u_DirLight, normal, viewDir, albedo);

    for (int i = 0; i < u_PointLightCount; ++i) {
        result += computePointLight(u_PointLights[i], normal, v_FragPos, viewDir, albedo);
    }

    o_Color = vec4(result, 1.0);
}
