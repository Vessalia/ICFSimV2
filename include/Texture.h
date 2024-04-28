#pragma once

#include <glad/glad.h>
#include <string>

class Texture
{
public:
	unsigned int ID;

	std::string type;
	std::string path;

	Texture(const std::string& path, const std::string& type);
	Texture(unsigned int width, unsigned int height, int glComponent = GL_RGB, int glType = GL_UNSIGNED_BYTE);

	~Texture();

	void use(GLenum target) const;

	void resize(unsigned int width, unsigned int height) const;
};