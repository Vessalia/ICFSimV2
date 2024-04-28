#pragma once
#include "Texture.h"
#include <vector>

class Framebuffer
{
public:
    Framebuffer(unsigned int width, unsigned int height, int texComponent = GL_RGBA32F);

    ~Framebuffer();

    void use() const;

    void clear() const;

    void copyToBackbuffer() const;

    const Texture& getFrameTexture() const;

private:
    unsigned int ID;

    unsigned int mWidth;
    unsigned int mHeight;

    Texture mFrameTexture;
};
