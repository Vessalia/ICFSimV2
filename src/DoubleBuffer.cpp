#include <DoubleBuffer.h>

DoubleBuffer::DoubleBuffer(unsigned int width, unsigned int height, int texComponent)
{
	mBuffers[0] = new Framebuffer(width, height, texComponent);
	mBuffers[1] = new Framebuffer(width, height, texComponent);
}

DoubleBuffer::~DoubleBuffer()
{
	delete mBuffers[0];
	delete mBuffers[1];
}

Framebuffer& DoubleBuffer::getRead()
{
	return *mBuffers[mIndex];
}

Framebuffer& DoubleBuffer::getWrite()
{
	return *mBuffers[(mIndex + 1) % 2];
}

void DoubleBuffer::swap()
{
	mIndex = (mIndex + 1) % 2;
}