 #version 460 core
in vec2 texCoord;

out vec3 fragColor;

uniform float rdx;
uniform float dt;
uniform float dissipation;
uniform sampler2D u;
uniform sampler2D x;

vec3 bilerp(sampler2D map, vec2 pos, vec2 dim)
{
	pos = pos * dim;
	vec4 posT;
	posT.xy = pos - 1;
	posT.zw = posT.xy + 1;

	pos = pos / dim;
	posT = vec4(posT.xy / dim, posT.zw / dim);

	vec2 t = pos - posT.xy;

	vec4 tex11 = texture(x, posT.xy);
	vec4 tex21 = texture(x, posT.zy);
	vec4 tex12 = texture(x, posT.xw);
	vec4 tex22 = texture(x, posT.zw);

	return mix(mix(tex11, tex21, t.x), mix(tex12, tex22, t.x), t.y).rgb;
}

vec3 advect(sampler2D map)
{
	vec2 pos = texCoord - dt * rdx * texture(u, texCoord).xy; 
	vec2 dim = textureSize(map, 0);
	return dissipation * bilerp(map, pos, dim).rgb;
}

void main()
{ 
	fragColor = advect(x);
}