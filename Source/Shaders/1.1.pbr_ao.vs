#version 400
layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec2 aTexCoords;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

//out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out float ao_v;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    //TexCoords = aTexCoords;
    WorldPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(model) * aNormal;   
    ao_v = aColor.x;
    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}