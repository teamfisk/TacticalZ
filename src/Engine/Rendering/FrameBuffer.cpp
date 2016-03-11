
#include "Rendering/FrameBuffer.h"


BufferResource::BufferResource(GLuint* resourceHandle, GLenum resourceType, GLenum attachment, GLuint mipMapLod, bool multiSampling)
{
    m_ResourceHandle = resourceHandle;
    m_ResourceType = resourceType;
    m_Attachment = attachment;
    m_MipMapLod = mipMapLod;
	m_MultiSampling = multiSampling;
}

Texture2D::~Texture2D()
{
    if (m_ResourceHandle != 0) {
        glDeleteTextures(1, m_ResourceHandle);
    }
}

Texture2DMultiSample::~Texture2DMultiSample()
{
	if (m_ResourceHandle != 0) {
		glDeleteTextures(1, m_ResourceHandle);
	}
}

RenderBuffer::~RenderBuffer()
{
    if (m_ResourceHandle != 0) {
        glDeleteRenderbuffers(1, m_ResourceHandle);
    }
}

Texture2DArray::~Texture2DArray()
{
	if (m_ResourceHandle != 0) {
		glDeleteTextures(1, m_ResourceHandle);
	}
}


FrameBuffer::~FrameBuffer()
{
    if (m_BufferHandle != 0) {
        glDeleteFramebuffers(1, &m_BufferHandle);
    }
}

void FrameBuffer::AddResource(std::shared_ptr<BufferResource> resource)
{
	//m_MultiSampling is true when initialized
	if (m_MultiSampling != resource->m_MultiSampling) {
		if (m_MultiSampling == false) {
			GLERROR("All renderbuffers is/is not using multisampling");
		} else {
			m_MultiSampling = false;
		}
	}
    m_Resources.push_back(resource);
}

void FrameBuffer::Generate()
{
    GLERROR("PRE");

    std::vector<GLenum> attachments;
	if (m_BufferHandle == 0) {
		glGenFramebuffers(1, &m_BufferHandle);
	}
    glBindFramebuffer(GL_FRAMEBUFFER, m_BufferHandle);
    GLERROR("1");
	for (auto it = m_Resources.begin(); it != m_Resources.end(); it++) {
		switch ((*it)->m_ResourceType) {
		case GL_TEXTURE_2D_MULTISAMPLE:
			glFramebufferTexture2D(GL_FRAMEBUFFER, (*it)->m_Attachment, GL_TEXTURE_2D_MULTISAMPLE, 0, 0);
			GLERROR("FrameBuffer generate: GL_TEXTURE_2D_MULTISAMPLE");
			break;
		case GL_TEXTURE_2D:
			glFramebufferTexture(GL_FRAMEBUFFER, (*it)->m_Attachment, *(*it)->m_ResourceHandle, (*it)->m_MipMapLod);
			GLERROR("FrameBuffer generate: GL_TEXTURE_2D");
			break;
		case GL_RENDERBUFFER:
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, (*it)->m_Attachment, (*it)->m_ResourceType, *(*it)->m_ResourceHandle);
			GLERROR("FrameBuffer generate: GL_RENDERBUFFER");
			break;
		case GL_TEXTURE_2D_ARRAY:
			glFramebufferTexture(GL_FRAMEBUFFER, (*it)->m_Attachment, *(*it)->m_ResourceHandle, 0);
			GLERROR("FrameBuffer generate: GL_TEXTURE_2D_ARRAY");
			break;
		}
		GLERROR("2");

		if ((*it)->m_Attachment != GL_DEPTH_ATTACHMENT && (*it)->m_Attachment != GL_STENCIL_ATTACHMENT && (*it)->m_Attachment != GL_DEPTH_STENCIL_ATTACHMENT) {
			attachments.push_back((*it)->m_Attachment);
		}
		GLERROR("Attachment");
	}

	GLERROR("3");

    GLenum* bufferTextures = attachments.data();
    glDrawBuffers(attachments.size(), bufferTextures);
    if (GLERROR("GLBufferAttachement error")) {
        printf(": AttachmentSize %i", attachments.size());
    }

    if (GLenum frameBufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        GLERROR("Framebuffer incomplete");
        LOG_ERROR("FrameBuffer incomplete: 0x%x\n", glCheckFramebufferStatus(GL_FRAMEBUFFER));
        exit(EXIT_FAILURE);
    }
    GLERROR("END");

}

void FrameBuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_BufferHandle);
}

void FrameBuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint FrameBuffer::GetHandle()
{
    return m_BufferHandle;
}

void FrameBuffer::Read() {
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_BufferHandle);
}

void FrameBuffer::Draw() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_BufferHandle);
}
