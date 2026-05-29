#version 330 core

in vec4 objectColour;
in vec2 objectTexCoord;
in vec3 objectNormal;
in vec3 afragPos;
out vec4 fragColor;

uniform sampler2D ourTexture;
uniform vec3 lightColour;
uniform vec3 lightColour2;
uniform vec3 lightPos;
uniform vec3 lightPos2;
uniform bool isEmissive;
uniform vec3 eyeDirection;

// Calcuate the Phong diffusion and specular, scaled by strength
vec3 phong(vec3 lp, vec3 lc, vec3 n, vec3 v, vec3 fragPos, float strength) {
    vec3 l = normalize(lp - fragPos);

    // difussion
    float kd = 0.7;
    vec3 diffuse = kd * max(dot(l, n), 0.0) * lc;

    // speculer
    float ks = 0.9;
    vec3 h = normalize(l + v);
    float shininess = 64.0;
    vec3 specular = ks * pow(max(dot(h, n), 0.0), shininess) * lc;

    return strength * (diffuse + specular);
}
void main() 
{   
    // texture
    vec3 material = texture(ourTexture, objectTexCoord).rgb;

    vec3 result;

    if (isEmissive)
    {
        // if the object is the light source, then it should just emit the full bright colour
        result = material;
    }
    else {
        // ambient
        float ambientStrength = 0.2;
        // use a weighted avearage for ambient colour
        vec3 ambientColour = (0.6 * lightColour) + (0.4 * lightColour2);
        vec3 ambient = ambientStrength * ambientColour;

        vec3 n = normalize(objectNormal);
        vec3 v = normalize(eyeDirection - afragPos);

        // Diffuse + specular per light

        vec3 light1 = phong(lightPos,  lightColour,  n, v, afragPos, 0.6);
        vec3 light2 = phong(lightPos2, lightColour2, n, v, afragPos, 0.4);

        result = (ambient + light1 + light2) * material;

    }   
    fragColor = vec4(result, 1.0);
}
