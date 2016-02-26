#ifndef FrameBuffer_h__
#define FrameBuffer_h__

#include "../OpenGL.h"
#include "Util/GLError.h"

class BufferResource
{
public:
    BufferResource(GLuint* resourceHandle, GLenum resourceType, GLenum attachment);
	BufferResource(GLuint* resourceHandle, GLenum resourceType, GLenum attachment, GLuint layers);
    
    GLuint* m_ResourceHandle;
    GLenum m_ResourceType;
    GLenum m_Attachment;
	GLuint m_Layers;
private:
    
};

template <GLenum RESOURCETYPE>
class ResourceType : public BufferResource
{
public:
    ResourceType(GLuint* resourceHandle, GLenum attachment)
        : BufferResource(resourceHandle, RESOURCETYPE, attachment) { }

	ResourceType(GLuint* resourceHandle, GLenum attachment, GLuint layers)
		: BufferResource(resourceHandle, RESOURCETYPE, attachment, layers) { }
};

class Texture2D : public ResourceType<GL_TEXTURE_2D>
{
public:
    Texture2D(GLuint* resourceHandle, GLenum attachment)
        : ResourceType(resourceHandle, attachment) { };

    ~Texture2D();
};

class RenderBuffer : public ResourceType<GL_RENDERBUFFER>
{
public:
    RenderBuffer(GLuint* resourceHandle, GLenum attachment)
        : ResourceType(resourceHandle, attachment)
    { };

    ~RenderBuffer();
};

class Texture2DArray : public ResourceType<GL_TEXTURE_2D_ARRAY>
{
public:
	Texture2DArray(GLuint* resourceHandle, GLenum attachment, GLuint layers)
		: ResourceType(resourceHandle, attachment, layers) { };

	~Texture2DArray();
};

class FrameBuffer
{
public:
    FrameBuffer()
        : m_BufferHandle(0) { }
    ~FrameBuffer();

    void AddResource(std::shared_ptr<BufferResource> resource);
    void Generate();
    void Bind();
    void Unbind();
    GLuint GetHandle();

private:
    GLuint m_BufferHandle;
    std::vector<std::shared_ptr<BufferResource>> m_Resources;
};

#endif
