#version 330 core

layout(location = 0) in vec3 vertPos;

uniform float uTime;


uniform mat4 P;
uniform mat4 MV;

void main()
{
	gl_Position = P * MV * vec4(vertPos + vec3(sin(uTime), cos(uTime), 0.0) * 0.1 * vertPos.z, 1.0);
}
