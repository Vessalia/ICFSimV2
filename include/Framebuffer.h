#pragma once
#include "Texture.h"
#include <vector>
#include <glm/glm.hpp>

class Framebuffer
{
public:
    Framebuffer(unsigned int width, unsigned int height, int texComponent = GL_RGBA32F);

    ~Framebuffer();

    void use() const;

    void clear() const;

    void copyToBackbuffer() const;
    void copyToBackbuffer(const glm::vec4& dest) const;

    const Texture& getFrameTexture() const;

private:
    unsigned int ID;

    unsigned int mWidth;
    unsigned int mHeight;

    Texture mFrameTexture;
};
