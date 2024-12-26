#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inTex;
layout(location = 2) in vec3 inNor;


out vec3 chNor;
out vec2 chTex;
out vec3 chFragPos;

uniform mat4 uM;
uniform mat4 uP;
uniform mat4 uV;


void main()
{
    chFragPos = vec3(uM * vec4(inPos, 1.0));
	chNor = mat3(transpose(inverse(uM))) * inNor;
	gl_Position = uP * uV * vec4(chFragPos,1.0);
    chTex = vec2((inPos.x + 1.0) * 0.5, (inPos.z + 1.0) * 0.5);
}