#version 460 core
in vec2 texCoord;

out vec3 fragColor;

uniform float halfrdx;
uniform sampler2D mapP;
uniform sampler2D mapW;

vec4 get(vec2 offset)
{
    return texture2D(mapP, texCoord + offset);
}

void main()
{
	vec2 dim = textureSize(mapP, 0).xy;

	vec4 pL = get(vec2(-1,  0) / dim);
	vec4 pR = get(vec2( 1,  0) / dim);
	vec4 pB = get(vec2( 0, -1) / dim);
	vec4 pT = get(vec2( 0,  1) / dim);

	vec2 grad = vec2((pR - pL).x, (pT - pB).y) * halfrdx;

	fragColor = texture(mapW, texCoord).rgb - vec3(grad, 0);
}