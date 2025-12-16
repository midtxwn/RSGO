#version 330 core

in vec4 ourColor;
in vec3 ourNorm;
in vec2 ourText;
in vec3 fragPos;

uniform sampler2D texture1;
uniform float alphaChannelFactor;

layout(location = 0) out vec4 FragColor;

void main()
{
/*
gl_FragColor = ourColor;
if(ourColor == vec4(0,0,0,0)) gl_FragColor = vec4(ourNorm, 1.f);
*/

FragColor = texture(texture1, ourText) * vec4(1.f, 1.f, 1.f, alphaChannelFactor);
}