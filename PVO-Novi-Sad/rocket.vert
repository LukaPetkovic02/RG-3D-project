#version 330 core

layout (location = 0) in vec3 aPosition;

uniform vec2 uTranslation;
uniform mat4 uM;
uniform mat4 uP;
uniform mat4 uV;

void main()
{
    gl_Position = uP * uV * uM * vec4(aPosition.x + uTranslation.x, aPosition.y + 0.2, aPosition.z + uTranslation.y, 1.0); // Stavila sam da se na y doda 0.2 zbog niskoletnih meta
}
