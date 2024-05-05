#version 460 core
in vec2 texCoord;

out vec3 fragColor;

uniform float alpha;
uniform float rBeta;
uniform sampler2D x;
uniform sampler2D b;

vec4 get(vec2 offset)
{
    return texture2D(x, texCoord + offset);
}

void main()
{
	vec2 dim = textureSize(x, 0).xy;

	vec4 xL = get(vec2(-1,  0) / dim);
	vec4 xR = get(vec2( 1,  0) / dim);
	vec4 xB = get(vec2( 0, -1) / dim);
	vec4 xT = get(vec2( 0,  1) / dim);

	vec4 bC = texture(b, texCoord);

	fragColor = vec3((xL + xR + xB + xT + alpha * bC) * rBeta);
}