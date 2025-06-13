#version 330 core

struct Material {
    vec3 kA;
    vec3 kD;
    vec3 kS;
    float shine;
};

struct Light {
    vec3 pos;   
    vec3 dir;  
    float cutoff;
    vec3 kA; 
    vec3 kD;
    vec3 kS;      
};

struct PointLight {
    vec3 pos;
    vec3 color;
    float intensity;
    float cutoff;
};


in vec3 chFragPos;
in vec3 chNor;

out vec4 outCol;

uniform vec3 color;
uniform float uAlpha;
uniform sampler2D uTex;

uniform Light uReflector;
uniform PointLight uHelicopterLights[5];

uniform Material uMaterial;
uniform vec3 uViewPos;

void main()
{
    vec3 resA = vec3(0.2);

    vec3 normal = normalize(chNor);
    
    vec3 lightDirection = normalize(vec3(0.0, 1.8, 0.0)); // Mesecina
    
    float nD = max(dot(normal, lightDirection), 0.0);
    vec3 resD = vec3(0.5) * (nD * uMaterial.kD);

    vec3 viewDirection = normalize(uViewPos - chFragPos);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float s = pow(max(dot(viewDirection, reflectionDirection), 0.0), uMaterial.shine);
    vec3 resS = vec3(0.2) * (s * uMaterial.kS);

    vec3 finalColor = resD + resS;

    // Reflektor
        vec3 lightDir = normalize(uReflector.pos - chFragPos);
        float spotCosine = dot(-lightDir, normalize(uReflector.dir));
        float spotFactor =  step(uReflector.cutoff, spotCosine);
        float nDReflector = max(dot(normal, lightDir), 0.0);
        vec3 resDReflector = spotFactor * uReflector.kD * (nDReflector * uMaterial.kD);
        vec3 viewDir = normalize(uViewPos - chFragPos);
        vec3 reflectDir = reflect(-lightDir, normal);
        float specular = pow(max(dot(viewDir, reflectDir), 0.0), uMaterial.shine);
        vec3 resSReflector = spotFactor * uReflector.kS * (specular * uMaterial.kS);

        vec3 finalColorReflector = resDReflector + resSReflector;
    //
//    if(finalColorReflector.x>0.0f){
//        outCol = vec4(1.0f, 0.0f, 0.0f, 1.0f);
//        return;
//    }

    // Tackasto svjetlo helikoptera
//    vec3 lightDirHeli = normalize(uHelicopterLight.pos - chFragPos);
//    float distanceToLight = length(uHelicopterLight.pos - chFragPos);
//    float attenuation = 1.0 / (1.0 + distanceToLight * uHelicopterLight.intensity);
//    float nDHeli = max(dot(normal, lightDirHeli), 0.0);
//    vec3 resDHeli = attenuation * uHelicopterLight.color * (nDHeli * uMaterial.kD);
//    vec3 reflectDirHeli = reflect(-lightDirHeli, normal);
//    float specularHeli = pow(max(dot(viewDirection, reflectDirHeli), 0.0), uMaterial.shine);
//    vec3 resSHeli = attenuation * uHelicopterLight.color * (specularHeli * uMaterial.kS);
//
//    vec3 finalColorHeli = resDHeli + resSHeli;

    vec3 finalColorHeli = vec3(0.0);
    for (int i = 0; i < 5; i++) {
        if (uHelicopterLights[i].intensity > 0.0) { // Provera da li je svjetlo aktivno
            
            vec3 lightDirHeli = normalize(uHelicopterLights[i].pos - chFragPos);
            float distanceToLight = length(uHelicopterLights[i].pos - chFragPos);
            float attenuation = 1.0 / (1.0 + distanceToLight * uHelicopterLights[i].intensity);
            float nDHeli = max(dot(normal, lightDirHeli), 0.0);
            vec3 resDHeli = attenuation * uHelicopterLights[i].color * (nDHeli * uMaterial.kD);
            vec3 reflectDirHeli = reflect(-lightDirHeli, normal);
            float specularHeli = pow(max(dot(viewDirection, reflectDirHeli), 0.0), uMaterial.shine);
            vec3 resSHeli = attenuation * uHelicopterLights[i].color * (specularHeli * uMaterial.kS);
            if(distanceToLight > 2.0){
                continue;
            }
            finalColorHeli += resDHeli + resSHeli;
        }
    }

    outCol = vec4(color * (resA + finalColor + finalColorReflector + finalColorHeli), 1.0 - uAlpha);
}
