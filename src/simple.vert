#version 330 core

in vec3 position;
uniform vec4 colour;
in vec2 TexCoord;
in vec3 normal;

out vec4 objectColour;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
out vec2 objectTexCoord;
out vec3 objectNormal;
out vec3 afragPos;

void main()
{   
    vec4 worldPos = model * vec4(position, 1.0);
    afragPos = worldPos.xyz;
    gl_Position = projection * view * worldPos;
    
    objectColour = colour;
    objectTexCoord = TexCoord;
    objectNormal = mat3(model) * normal; // need to transform the normal as well because the objects move in world space

}
