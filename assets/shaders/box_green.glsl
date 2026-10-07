#version 330 core

struct Light {
    vec3 color;
    vec3 direction;
};

uniform Light uLight;
uniform vec3 uCameraPos;
uniform vec3 color;
uniform vec3 box_green;

out vec4 FragColor;

in vec2 vUV;
in vec3 vNormal;
in vec3 vFragPos;

uniform sampler2D baseColorTexture;

void main() {
    vec3 norm = normalize(vNormal);

    //  ambient
    const float ambientStrength = 0.4;
    vec3 ambient = ambientStrength * uLight.color;

    //  diffuse
    vec3 lightDir = normalize(-uLight.direction);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * uLight.color;

    //  specular
    vec3 viewDir = normalize(uCameraPos - vFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    float specularStrength = 0.5;
    vec3 specular = specularStrength * spec * uLight.color;

    vec4 texColor = texture(baseColorTexture, vUV);
    vec3 result = (diffuse + specular + ambient) * texColor.xyz * box_green;
    FragColor = vec4(result, 1.0);
}