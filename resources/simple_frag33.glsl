#version 330 core

uniform vec2 uWindowSize;
uniform sampler2D uTexture;

out vec3 color;

void main()
{
	color = vec3(0.0, 0.0, 1.0);

	color.r = gl_FragCoord.x / uWindowSize.x;

	if (length(gl_FragCoord.xy - uWindowSize / 2.0) < 45.0)
	{
		color.g = 1.0;
	}

	color.rgb *= texture(uTexture, gl_FragCoord.xy / uWindowSize * vec2(1.0, -1.0)).rgb;
}
