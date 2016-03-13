#ifndef FrameBuffer_h__
#define FrameBuffer_h__

#include "../OpenGL.h"
#include "Util/GLError.h"

class BufferResource
{
public:
    BufferResource(GLuint* resourceHandle, GLenum resourceType, GLenum attachment, GLuint mipMapLod, bool multiSampling);
    
    GLuint* m_ResourceHandle;
    GLenum m_ResourceType;
    GLenum m_Attachment;
    GLuint m_MipMapLod = 0;
	bool m_MultiSampling = false;
private:
    
};

template <GLenum RESOURCETYPE>
class ResourceType : public BufferResource
{
public:
    ResourceType(GLuint* resourceHandle, GLenum attachment, GLuint mipMapLod, bool multiSampling)
        : BufferResource(resourceHandle, RESOURCETYPE, attachment, mipMapLod, multiSampling) { }
};

class Texture2D : public ResourceType<GL_TEXTURE_2D>
{
public:
    Texture2D(GLuint* resourceHandle, GLenum attachment, GLuint mipMapLod = 0)
        : ResourceType(resourceHandle, attachment, mipMapLod, false) { };

    ~Texture2D();
};

class Texture2DMultiSample : public ResourceType<GL_TEXTURE_2D_MULTISAMPLE>
{
public:
	Texture2DMultiSample(GLuint* resourceHandle, GLenum attachment, GLuint mipMapLod = 0)
		: ResourceType(resourceHandle, attachment, mipMapLod, true) { };

	~Texture2DMultiSample();
};

class RenderBuffer : public ResourceType<GL_RENDERBUFFER>
{
public:
    RenderBuffer(GLuint* resourceHandle, GLenum attachment, bool multiSampling = false)
        : ResourceType(resourceHandle, attachment, 0, multiSampling)
    { };

    ~RenderBuffer();
};

class Texture2DArray : public ResourceType<GL_TEXTURE_2D_ARRAY>
{
public:
	Texture2DArray(GLuint* resourceHandle, GLenum attachment)
		: ResourceType(resourceHandle, attachment, 0, false)
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
	void Read();
	void Draw();
    GLuint GetHandle();
	bool MultiSampling() { return m_MultiSampling; };

private:
	bool m_MultiSampling = true;
    GLuint m_BufferHandle;
    std::vector<std::shared_ptr<BufferResource>> m_Resources;
};

#endif
