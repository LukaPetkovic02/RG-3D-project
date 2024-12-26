#version 330 core

out vec4 outCol;
uniform vec3 color;

void main()
{
	outCol = vec4(color, 1.0);
}