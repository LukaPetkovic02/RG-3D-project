#version 330 core

out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D uTexture;

void main() {

    vec4 uColor = vec4(0.0, 0.0, 0.0, 0.4);
    vec4 texColor = texture(uTexture, TexCoord);
    vec4 finalColor = (texColor.a > 0.0) ? texColor : uColor;
    FragColor = finalColor;
}
