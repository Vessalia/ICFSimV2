#include "Framebuffer.h"
#include "iostream"
#include <VertexData.h>
#include "Constants.h"

Framebuffer::Framebuffer(unsigned int width, unsigned int height, int texComponent)
    : mFrameTexture(Texture(width, height, texComponent, GL_FLOAT))
    , mWidth(width)
    , mHeight(height)
{
    glGenFramebuffers(1, &ID);
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mFrameTexture.ID, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    clear();
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &ID);
}

void Framebuffer::use() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, ID);
    glViewport(0, 0, mWidth, mHeight);
}

void Framebuffer::clear() const
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Framebuffer::copyToBackbuffer() const
{
    glBlitNamedFramebuffer(ID, 0, 0, 0, mWidth, mHeight, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

const Texture& Framebuffer::getFrameTexture() const
{
    return mFrameTexture;
}
