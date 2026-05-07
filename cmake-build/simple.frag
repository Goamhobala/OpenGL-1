#version 330 core

uniform vec3 objectColor;
in vec2 TextureCoord;

out vec4 fragColor;

uniform sampler2D ourTexture;
void main()
{
    fragColor = texture(ourTexture, TextureCoord);
}
