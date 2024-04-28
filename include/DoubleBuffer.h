#pragma once
#include <Framebuffer.h>

class DoubleBuffer
{
public:
	DoubleBuffer(unsigned int width, unsigned int height, int texComponent = GL_RGBA32F);

	~DoubleBuffer();

	Framebuffer& getRead();
	Framebuffer& getWrite();

	void swap();

private:
	int mIndex = 0;
	Framebuffer* mBuffers[2];
};
