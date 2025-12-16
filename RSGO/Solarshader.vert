#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNorm;
layout (location = 2) in vec2 aText;
layout (location = 3) in vec4 aColor;

out vec4 ourColor;
out vec3 ourNorm;
out vec2 ourText;
out vec3 fragPos;

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

void main()
{
   gl_Position = projection * view * transform * vec4(aPos, 1.0);
   ourColor = aColor;
   //ourNorm = aNorm;
   ourText = aText;

   ourNorm = mat3(transpose(inverse(transform))) * aNorm;
   fragPos = vec3(transform * vec4(aPos, 1.0));
}