#version 330 core

in vec3 position;
in vec3 colour;
in vec2 texture;

out vec3 Colour;
out vec2 TextureCoord;

void main()
{
    gl_Position = vec4(position,1.0f);
    Colour = colour;
    TextureCoord = texture;
}
