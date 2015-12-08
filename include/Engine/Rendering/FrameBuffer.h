#ifndef FrameBuffer_h__
#define FrameBuffer_h__

#include "../OpenGL.h"
#include "Util/GLError.h"

class BufferResource
{
public:
    BufferResource(GLuint* resourceHandle, GLenum resourceType, GLenum attachment);
    
    GLuint* m_ResourceHandle;
    GLenum m_ResourceType;
    GLenum m_Attachment;
private:
    
};

template <GLenum RESOURCETYPE>
class ResourceType : public BufferResource
{
public:
    ResourceType(GLuint* resourceHandle, GLenum attachment)
        : BufferResource(resourceHandle, RESOURCETYPE, attachment) { }
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
