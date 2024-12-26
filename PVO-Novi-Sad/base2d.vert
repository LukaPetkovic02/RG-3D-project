#version 330 core

layout(location = 0) in vec2 inPos;


uniform vec2 translation;
uniform bool applyTranslation;

void main()
{
	if (applyTranslation)
        gl_Position = vec4(inPos + translation, -1.0, 1.0);
    else
        gl_Position = vec4(inPos, -1.0, 1.0); // Nema translacije
}