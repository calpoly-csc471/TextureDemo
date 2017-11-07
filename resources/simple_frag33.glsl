#version 330 core

uniform vec2 uWindowSize;
uniform sampler2D uTexture;

in vec2 fTexCoords;

out vec3 color;

void main()
{
	color = texture(uTexture, fTexCoords * vec2(1.0, -1.0)).rgb;
}
