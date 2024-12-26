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


in vec3 chFragPos;
in vec3 chNor;

out vec4 outCol;

uniform vec3 color;
uniform float uAlpha;
uniform sampler2D uTex;

uniform Light uReflector;

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
    outCol = vec4(color * (resA + finalColor + finalColorReflector), 1.0 - uAlpha);
}
