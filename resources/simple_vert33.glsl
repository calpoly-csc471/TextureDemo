#version 330 core

layout(location = 0) in vec3 vertPos;

uniform mat4 P;
uniform mat4 MV;

out vec2 fTexCoords;

void main()
{
	gl_Position = P * MV * vec4(vertPos, 1.0);
	fTexCoords = vertPos.xy / 2.0 + vec2(0.5);
}
