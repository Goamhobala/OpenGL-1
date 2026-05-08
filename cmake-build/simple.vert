#version 330 core

in vec3 position;
uniform vec4 colour;

out vec4 objectColour;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position,1.0f);
    
    objectColour = colour;
}
