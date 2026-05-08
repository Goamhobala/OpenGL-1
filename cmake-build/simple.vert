#version 330 core

in vec3 position;
in vec3 colour;
in vec2 texture;

out vec3 Colour;
out vec2 TextureCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position,1.0f);
    
    Colour = colour;
    TextureCoord = texture;
}
