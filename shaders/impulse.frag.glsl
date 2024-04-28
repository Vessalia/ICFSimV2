#version 460 core
in vec2 texCoord;

out vec3 fragColor;

uniform float invR;
uniform vec2 impulsePos;
uniform vec4 Fdt; // force for velocity, colour for ink
uniform sampler2D base;

vec3 calcForce(vec2 pos, float invRad)
{
	return Fdt.xyz * exp(-dot(pos, pos) * invRad);
}

void main()
{
	vec2 pos = impulsePos - texCoord;
	vec3 color = calcForce(pos, invR);

	fragColor = texture(base, texCoord).rgb + color;
}