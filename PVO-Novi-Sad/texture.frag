#version 330 core

in vec2 chTex;
out vec4 outCol;

struct Material {
    vec3 kA;
    vec3 kD;
    vec3 kS;
    float shine;
};

in vec3 chFragPos;
in vec3 chNor;

uniform sampler2D uTex;
uniform Material uMaterial;
uniform vec3 uViewPos;

void main()
{
    vec4 texColor = texture(uTex, chTex)* vec4(1.0);

    vec3 resA = vec3(0.1);

    vec3 normal = normalize(chNor);
    
    vec3 lightDirection = normalize(vec3(0.0, 1.2, 0.0)); // Mesecina
    
    float nD = max(dot(normal, lightDirection), 0.0);
    vec3 resD = vec3(0.5) * (nD * uMaterial.kD);

    vec3 viewDirection = normalize(uViewPos - chFragPos);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float s = pow(max(dot(viewDirection, reflectionDirection), 0.0), uMaterial.shine);
    vec3 resS = vec3(0.2) * (s * uMaterial.kS);

    vec3 finalColor = resA + resD + resS;

    outCol = vec4(vec3(texColor) * finalColor, 1.0);
}