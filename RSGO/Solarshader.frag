#version 330 core

uniform float screen_width;
uniform float screen_height;

in vec4 ourColor;
in vec3 ourNorm;
in vec2 ourText;

in vec3 fragPos;


uniform vec4 objectColor;
uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform sampler2D texture1;

layout(location = 0) out vec4 FragColor;

void main()
{
/*
gl_FragColor = ourColor;
if(ourColor == vec4(0,0,0,0)) gl_FragColor = vec4(ourNorm, 1.f);
*/



//MIRA   //Entre ancho 2*longitudMira y alto 2*grosorMira o entre alto 2*longitudMira y ancho 2*grosorMira
float grosorMira = 1.5, longitudMira = 12;
float middleWidth = screen_width/2;
float middleHeight = screen_height/2;
if(
	gl_FragCoord.x > (middleWidth - longitudMira) && gl_FragCoord.x < (middleWidth + longitudMira) &&
	gl_FragCoord.y > (middleHeight - grosorMira) && gl_FragCoord.y < (middleHeight + grosorMira)
	||
	gl_FragCoord.y > (middleHeight - longitudMira) && gl_FragCoord.y < (middleHeight + longitudMira) &&
	gl_FragCoord.x > (middleWidth - grosorMira) && gl_FragCoord.x < (middleWidth + grosorMira)
	)
	{
		FragColor = vec4(1.f, 1.f, 1.f, 1.f);
		return;
	}
	




//LUCES:cc
float alphaLight = lightColor[3];
vec3 lColor = vec3(lightColor[0], lightColor[1], lightColor[2]);

	//ambient
float ambientI = 0.3; //0.3
vec3 ambient = ambientI * lColor;

	//diffuse
vec3 norm = normalize(ourNorm);
vec3 lightDir = normalize(lightPos - fragPos);
float diff = max(dot(norm, lightDir), 0.);
vec3 diffuse = diff * lColor;

		//Evita especular del lado opuesto
		
if(diff == 0){
	//gl_FragColor = vec4(ambient[0], ambient[1], ambient[2], alphaLight) * objectColor;
	FragColor =  vec4(ambient[0], ambient[1], ambient[2], alphaLight) * texture(texture1, ourText);
	return;
}

	//specular
float specularS = 1.0;
vec3 viewDir = normalize(viewPos - fragPos);
vec3 reflectDir = reflect(-lightDir, norm);
float spec = pow(max(dot(viewDir, reflectDir), 0.0), 2); //TODO: el 128 pasarselo al shader
vec3 specular = specularS * spec * lColor;

vec3 resLight = ambient + diffuse + specular;
//gl_FragColor = vec4(resLight[0], resLight[1], resLight[2], alphaLight) * objectColor;

FragColor = vec4(resLight[0], resLight[1], resLight[2], alphaLight) * texture(texture1, ourText);

//vec3 partial = vec3(texture(texture1, ourText));
//gl_FragColor = vec4(partial[0], partial[1], partial[2], 0.0f);
}