#ifndef FrameBuffer_h__
#define FrameBuffer_h__

#include "../OpenGL.h"
#include "Util/GLError.h"

class BufferResource
{
public:
    BufferResource(GLuint* resourceHandle, GLenum resourceType, GLenum attachment, GLuint mipMapLod);
    
    GLuint* m_ResourceHandle;
    GLenum m_ResourceType;
    GLenum m_Attachment;
    GLuint m_MipMapLod = 0;
private:
    
};

template <GLenum RESOURCETYPE>
class ResourceType : public BufferResource
{
public:
    ResourceType(GLuint* resourceHandle, GLenum attachment, GLuint mipMapLod)
        : BufferResource(resourceHandle, RESOURCETYPE, attachment, mipMapLod) { }
};

class Texture2D : public ResourceType<GL_TEXTURE_2D>
{
public:
    Texture2D(GLuint* resourceHandle, GLenum attachment, GLuint mipMapLod = 0)
        : ResourceType(resourceHandle, attachment, mipMapLod) { };

    ~Texture2D();
};

class RenderBuffer : public ResourceType<GL_RENDERBUFFER>
{
public:
    RenderBuffer(GLuint* resourceHandle, GLenum attachment)
        : ResourceType(resourceHandle, attachment, 0)
    { };

    ~RenderBuffer();
};

class Texture2DArray : public ResourceType<GL_TEXTURE_2D_ARRAY>
{
public:
	Texture2DArray(GLuint* resourceHandle, GLenum attachment)
		: ResourceType(resourceHandle, attachment, 0)
	{ };

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
