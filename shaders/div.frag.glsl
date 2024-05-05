#version 460 core
in vec2 texCoord;

out vec3 fragColor;

uniform float halfrdx;
uniform sampler2D map;

vec4 get(vec2 offset)
{
    return texture2D(map, texCoord + offset);
}

void main()
{
	vec2 dim = textureSize(map, 0).xy;

	vec4 xL = get(vec2(-1,  0) / dim);
	vec4 xR = get(vec2( 1,  0) / dim);
	vec4 xB = get(vec2( 0, -1) / dim);
	vec4 xT = get(vec2( 0,  1) / dim);

	fragColor = vec3((xR.x - xL.x + xT.y - xB.y) * halfrdx);
}