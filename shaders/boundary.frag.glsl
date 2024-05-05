#version 460 core
in vec2 texCoord;

out vec3 fragColor;

uniform int scale;
uniform sampler2D x;

void main()
{
	vec2 invDim = 1 / textureSize(x, 0);

	vec2 offset = vec2(0, 0);
	if(texCoord.x <= invDim.x)
	{
		offset.x = 1;
	}
	else if(texCoord.x >= 1 - invDim.x)
	{
		offset.x = -1;
	}

	if(texCoord.y >= 1 - invDim.y)
	{
		offset.y = -1;
	}
	else if(texCoord.y <= invDim.y)
	{
		offset.y = 1;
	}

	offset *= scale * invDim;

	fragColor = texture(x, texCoord + offset).rgb;
}