#version 330 core

layout (location = 0) in vec2 aPosition;

uniform vec2 uTranslation;
uniform float uScale;

void main()
{
    // Skaliramo koordinate na osnovu faktora
    vec2 scaledPosition = aPosition * uScale;
    gl_Position = vec4(scaledPosition + uTranslation, -1.0, 1.0);
    //gl_Position = vec4(aPosition + uTranslation, -1.0, 1.0);//staro
}