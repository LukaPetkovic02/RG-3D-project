#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNor;

uniform vec3 uTranslation;

out vec3 chNor;
out vec3 chFragPos;

uniform mat4 uM;
uniform mat4 uP;
uniform mat4 uV;

void main()
{
	
	//chTex = vec2((inPos.x + 1.0) * 0.5, (inPos.z + 1.0) * 0.5);
	chFragPos = vec3(uM * vec4(inPos + vec3(uTranslation.x, uTranslation.y, uTranslation.z), 1.0));
	chNor = mat3(transpose(inverse(uM))) * inNor;
	gl_Position = uP * uV * vec4(chFragPos,1.0); 
}