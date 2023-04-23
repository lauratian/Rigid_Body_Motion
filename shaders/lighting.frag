#version 330 core

in vec4 position; // raw position in the model coord
in vec3 normal;   // raw normal in the model coord

uniform mat4 modelview; // from model coord to eye coord
uniform mat4 view;      // from world coord to eye coord

// Material parameters
uniform vec4 ambient;
uniform vec4 diffuse;
uniform vec4 specular;
uniform vec4 emision;
uniform float shininess;

// Light source parameters
const int maximal_allowed_lights = 10;
uniform bool enablelighting;
uniform int nlights;
uniform vec4 lightpositions[ maximal_allowed_lights ];
uniform vec4 lightcolors[ maximal_allowed_lights ];

// Output the frag color
out vec4 fragColor;


void main (void){
    if (!enablelighting){
        // Default normal coloring (you don't need to modify anything here)
        vec3 N = normalize(normal);
        fragColor = vec4(0.5f*N + 0.5f, 1.0f);
    } else {
        // HW3: You will compute the lighting here.
        fragColor = emision;                                        
        vec4 eyePosition = vec4(0.0, 0.0, 0.0, 1.0);                //the eye position is not changing
        vec4 p = modelview * position;                              //point surface position in the camera coordinate
        
        vec3 v = vec3(normalize(eyePosition - p));                                //unit vector pointing to viewer changed based on the new point position
        vec3 n = normalize(inverse(transpose(mat3(modelview))) * normal);         //normal vector in cam coordinate
        // for loop to add up all the lights
        for (int j=0; j < maximal_allowed_lights; j++) { 
            
            //the light position is differrent for each light, thus put inside the loop
            vec4 q_j = view * lightpositions[j];                          //light point position in the camera coordinate
            
            vec3 l_j = normalize( p.w * q_j.xyz - q_j.w * p.xyz );           //unit vector pointing to light 
            vec3 h_j = normalize( v + l_j );                                //unit half way vector

            fragColor += (ambient + diffuse*max(dot(n,l_j),0) + specular*pow(max(dot(n,h_j),0),shininess)) * lightcolors[j];
        }

        
        
    }
}