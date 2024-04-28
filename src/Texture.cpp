#include "Texture.h"
#include <SDL.h>
#include <SDL_image.h>

Texture::Texture(const std::string& path, const std::string& type)
	: path(path)
	, type(type)
{
	SDL_Surface* loadedSurface = IMG_Load(path.c_str());

	glGenTextures(1, &ID);
	use(GL_TEXTURE0);

	int format = GL_RGB;
	if (loadedSurface->format->BytesPerPixel == 4)
	{
		format = GL_RGBA;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, format, loadedSurface->w, loadedSurface->h, 0, format, GL_UNSIGNED_BYTE, loadedSurface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	SDL_FreeSurface(loadedSurface);
}

Texture::Texture(unsigned int width, unsigned int height, int glComponent, int glType)
{
	glGenTextures(1, &ID);
	use(GL_TEXTURE0);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
	glDeleteTextures(1, &ID);
}

void Texture::resize(unsigned int width, unsigned int height) const
{

}

void Texture::use(GLenum target) const
{
	glActiveTexture(target);
	glBindTexture(GL_TEXTURE_2D, ID);
}